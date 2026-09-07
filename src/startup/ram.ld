/**
 * Linker script for STM32H7
 * For debug, executes from RAM
 *
 * RAM_EXEC : 512K @ 0x24000000 - vector table, code, rodata
 * DTCMRAM  : 128K @ 0x20000000 - data, BSS, stack
 */

ENTRY(reset_handler)

/* Top of DTCMRAM (initial stack pointer) */
_estack = ORIGIN(DTCMRAM) + LENGTH(DTCMRAM);

_Min_Stack_Size = 0x400;

MEMORY
{
  RAM_EXEC (xrw) : ORIGIN = 0x24000000, LENGTH = 512K
  DTCMRAM  (xrw) : ORIGIN = 0x20000000, LENGTH = 128K
  RAM_D2   (xrw) : ORIGIN = 0x30000000, LENGTH = 288K
  RAM_D3   (xrw) : ORIGIN = 0x38000000, LENGTH = 64K
  ITCMRAM  (xrw) : ORIGIN = 0x00000000, LENGTH = 64K
}

SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(4);

    KEEP(*(.isr_vector))

    . = ALIGN(4);
  } >RAM_EXEC

  .text :
  {
    . = ALIGN(4);

    *(.text)
    *(.text*)

    /* Functions explicitly placed in RAM */
    *(.RamFunc)
    *(.RamFunc*)

    . = ALIGN(4);
    _etext = .;
  } >RAM_EXEC

  .rodata :
  {
    . = ALIGN(4);

    *(.rodata)
    *(.rodata*)

    . = ALIGN(4);
  } >RAM_EXEC

  /*
   * Initial values for .data are stored in RAM_EXEC.
   * Startup copies them to DTCMRAM.
   */
  _sidata = LOADADDR(.data);

  .data :
  {
    . = ALIGN(4);
    _sdata = .;

    *(.data)
    *(.data*)

    . = ALIGN(4);
    _edata = .;
  } >DTCMRAM AT>RAM_EXEC

  .bss :
  {
    . = ALIGN(4);

    _sbss = .;
    __bss_start__ = _sbss;

    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);

    _ebss = .;
    __bss_end__ = .;
  } >DTCMRAM

  /*
   * Reserve minimum stack space.
   * Initial stack pointer is _estack.
   */
  ._user_stack :
  {
    . = ALIGN(8);

    . = . + _Min_Stack_Size;

    . = ALIGN(8);
  } >DTCMRAM

  .ARM.attributes 0 :
  {
    *(.ARM.attributes)
  }
}
