#!/usr/bin/env bash
#
# Build, flash and run the CAN loopback HITL on the connected Nucleo, then
# read the results from RAM over SWD (no UART needed).
#
# Exit status: 0 = all passed, 1 = failures, 2 = timeout / infrastructure error.
#
#   scripts/hitl.sh [timeout_seconds]     (default 30)

set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD="$ROOT/build/hitl"
ELF="$BUILD/test/hitl/hitl_can_loopback.elf"
TIMEOUT="${1:-30}"
GDB_PORT=3333

OPENOCD_PID=""
GDB_PID=""
WORK="$(mktemp -d)"

cleanup()
{
    [ -n "$GDB_PID" ]     && kill "$GDB_PID"     2>/dev/null
    if [ -n "$OPENOCD_PID" ]; then
        kill "$OPENOCD_PID" 2>/dev/null
        wait "$OPENOCD_PID" 2>/dev/null
    fi
    rm -rf "$WORK"
}
trap cleanup EXIT

echo "== build"
cmake -S "$ROOT" -B "$BUILD" -DBUILD_HITL=ON -DCMAKE_BUILD_TYPE=Release >/dev/null \
    && cmake --build "$BUILD" -j >"$WORK/build.log" 2>&1 \
    || { cat "$WORK/build.log"; echo "build failed"; exit 2; }

echo "== openocd"
openocd -f interface/stlink.cfg -f target/stm32h7x.cfg >"$WORK/openocd.log" 2>&1 &
OPENOCD_PID=$!

for _ in $(seq 1 20); do
    grep -q "Listening on port $GDB_PORT" "$WORK/openocd.log" && break
    kill -0 "$OPENOCD_PID" 2>/dev/null || { cat "$WORK/openocd.log"; echo "openocd exited"; exit 2; }
    sleep 0.5
done
grep -q "Listening on port $GDB_PORT" "$WORK/openocd.log" \
    || { cat "$WORK/openocd.log"; echo "openocd did not come up"; exit 2; }

cat >"$WORK/run.gdb" <<'G'
set pagination off
set confirm off
set print elements 0
target extended-remote :3333
monitor reset halt
load
monitor reset halt
# Freeze the IWDG while halted (DBGMCU_APB4FZ1.DBG_IWDG1) so inspecting the
# target cannot trigger a reset.
set *(unsigned int*)0x5C001054 = *(unsigned int*)0x5C001054 | (1<<18)
break hitl_finish
continue
# stopped on entry to hitl_finish(): execute `hitl_done = 1`
next
maintenance flush register-cache
printf "HITL_DONE=%u\n",  hitl_done
printf "HITL_PASS=%u\n",  hitl_pass
printf "HITL_FAIL=%u\n",  hitl_fail
set $i = 0
while $i < hitl_fail && $i < 32
  printf "HITL_FAILED %s (test_can_loopback.c:%u)\n", hitl_fails[$i].name, hitl_fails[$i].line
  set $i = $i + 1
end
monitor reset run
G

echo "== flash + run (timeout ${TIMEOUT}s)"
arm-none-eabi-gdb -q -batch -x "$WORK/run.gdb" "$ELF" >"$WORK/gdb.log" 2>&1 &
GDB_PID=$!

( sleep "$TIMEOUT"; kill "$GDB_PID" 2>/dev/null ) 2>/dev/null &
WATCHDOG=$!
wait "$GDB_PID" 2>/dev/null
GDB_PID=""
kill "$WATCHDOG" 2>/dev/null
wait "$WATCHDOG" 2>/dev/null

if ! grep -q "^HITL_DONE=1" "$WORK/gdb.log"; then
    tail -20 "$WORK/gdb.log"
    echo "HITL did not finish (timeout, or target failed to load)"
    exit 2
fi

grep "^HITL_" "$WORK/gdb.log"

FAILS="$(sed -n 's/^HITL_FAIL=//p' "$WORK/gdb.log")"
[ "$FAILS" = "0" ] && { echo "== PASS"; exit 0; }
echo "== FAIL"
exit 1
