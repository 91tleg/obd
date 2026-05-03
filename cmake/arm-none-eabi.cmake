set(CMAKE_SYSTEM_NAME      Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_C_COMPILER    arm-none-eabi-gcc)
set(CMAKE_ASM_COMPILER  arm-none-eabi-gcc)
set(CMAKE_OBJCOPY       arm-none-eabi-objcopy)
set(CMAKE_SIZE          arm-none-eabi-size)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

execute_process(
    COMMAND arm-none-eabi-gcc -print-sysroot
    OUTPUT_VARIABLE ARM_SYSROOT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(CPU_FLAGS
    -mcpu=cortex-m7
    -mthumb
    -mfpu=fpv5-d16
    -mfloat-abi=hard
    -ffreestanding
    -nostdlib
    --sysroot=${ARM_SYSROOT}
)

string(JOIN " " CPU_FLAGS_STR ${CPU_FLAGS})

set(CMAKE_C_FLAGS_INIT          "${CPU_FLAGS_STR}")
set(CMAKE_ASM_FLAGS_INIT        "${CPU_FLAGS_STR}")
set(CMAKE_EXE_LINKER_FLAGS_INIT
    "-mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard -nostdlib -nodefaultlibs -nostartfiles")
