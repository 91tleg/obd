An OBD-II diagnostic scanner running on the STM32H753ZI (Nucleo-H753ZI). Reads live vehicle data and fault codes over CAN or K-line, and displays them on a TFT or LCD.

---

## Hardware

- **MCU**: STM32H753ZI (ARM Cortex-M7, 480 MHz, 2 MB flash, 1 MB SRAM)  
The H7 was chosen for an earlier project that ran on-device OCR, a
camera-based calculator that captures an image of a handwritten or
printed math expression, recognizes the characters locally,
and evaluates the result. The compute demands of image capture, inference,
and expression parsing justified the H7's FPU, cache, and DTCM. This project reuses the same hardware. For OBD scanning the H7 is
overkill; a CAN FD capable M4 would be sufficient. But the way this project is structured swapping a board/MCU means only HAL/BSP changes are needed.
- **CAN**: FDCAN peripheral, requires an external CAN transceiver (e.g. SN65HVD230)
- **K-line**: UART + external K-line driver IC (e.g. MC33290)
- **Display**: ST7789 TFT (SPI) or HD44780 LCD (I2C), selected at build time
- **OBD connector**: standard OBD-II port (CAN on pins 6/14, K-line on pin 7)

---

## OBD Protocol Support

| Protocol | Standard | Vehicles |
|---|---|---|
| CAN / ISO 15765 | ISO 15765-2 transport, ISO 15765-4 OBD | 2008+ (mandatory) |
| KWP2000 / K-line | ISO 14230 | ~1996–2008 |

Protocol is selected at build time via `OBD_PROTOCOL=can` or `OBD_PROTOCOL=kwp2000`. The application layer is protocol-agnostic — both transports expose the same `obd_protocol.h` interface.

### CAN / CAN FD

The CAN implementation supports both classic CAN 2.0B (8-byte) and CAN FD (64-byte) frames using the STM32H7 FDCAN peripheral configured in bare-metal. ISO 15765-2 segmentation and reassembly handles messages of any length across Single Frame, First Frame, and Consecutive Frame types per the standard's Table 9 PCI layout.

### KWP2000

KWP2000 runs at 10,400 baud over a single K-line wire. The init sequence uses 5-baud bit-banging to wake the ECU, followed by standard UART.

---

## Architecture

```
app/                        Application — polls PIDs, displays data
drivers/protocol/obd/       OBD protocol abstraction
  ├── can/                  CAN (ISO 15765-2 + OBD layer)
  └── kwp2000/              KWP2000 (ISO 14230)
hal/                        Hardware abstraction (CAN, UART, SPI, I2C)
bsp/nucleo_h753zi/          Board support (clocks, GPIO, peripherals)
lib/                        Utilities
```

Each layer only depends on the one directly below it. The HAL owns registers, the BSP owns pins and clocks, drivers own protocol logic, the app owns application behaviour.

### Porting to a different MCU or board
 
The layered architecture is designed so that porting requires touching only the HAL and BSP, everything above is hardware-agnostic.
 
To add a new target:
 
1. **Add a BSP directory**: `bsp/<new_board>/` with its own `CMakeLists.txt`. Implement clock init, GPIO config, and peripheral wiring (CAN TX/RX pins, UART pins, SPI/I2C for display).
2. **Implement the HAL**: `hal/<mcu>/can.c`, `hal/<mcu>/uart.c`... for the new MCU's register set. The HAL interface (`hal/*.h`) stays unchanged. See below for more details.

3. **Pass the new board at build time**:
```bash
cmake -S . -B build/new_board \
      -DBOARD=new_board \
      -DOBD_PROTOCOL=can \
      -DCMAKE_BUILD_TYPE=Release
```
 
The drivers, protocol stack, application, and all tests above the HAL require no changes.

> **TODO (porting):** `hal/` currently contains a single STM32H7-specific implementation at the top level. If to port to a different MCU, restructure it to mirror the BSP pattern:
>
> ```
> hal/
> ├── can.h         # Interface, never changes
> ├── uart.h
> ├── spi.h
> ├── i2c.h
> └── stm32h7/      # MCU-specific implementation
>     ├── can.c
>     ├── uart.c
>     ├── spi.c
>     └── i2c.c
> ```
>
> Add a `MCU` CMake variable (e.g. `MCU=stm32h7`) and select the implementation subdirectory in `hal/CMakeLists.txt`. This keeps the second port entirely contained to a new `hal/<mcu>/` directory with no changes to anything above it.
 
---

## Build

Requires the ARM GNU Toolchain and CMake 3.20+.

```bash
# CAN
cmake -S . -B build/debug \
      -DOBD_PROTOCOL=can \
      -DBOARD=nucleo_h753zi \
      -DDISPLAY=st7789 \
      -DCMAKE_BUILD_TYPE=Debug

cmake --build build/debug

# KWP
cmake -S . -B build/kwp \
      -DOBD_PROTOCOL=kwp2000 \
      -DBOARD=nucleo_h753zi \
      -DDISPLAY=st7789 \
      -DCMAKE_BUILD_TYPE=Debug

cmake --build build/kwp
```

Flash with ST-Link.

---

## Testing

Tests run on the host, no HW required for unit and SIL.

```bash
cd test
mkdir build && cd build
cmke ..
make
ctest --output-on-failure
```

---

## Configuration

| CMake variable | Options | Default |
|---|---|---|
| `OBD_PROTOCOL` | `can`, `kwp2000` | `can` |
| `DISPLAY` | `st7789`, `hd44780` | `st7789` |
| `BOARD` | `nucleo_h753zi` | — |
| `CMAKE_BUILD_TYPE` | `Debug`, `Release` | — |
