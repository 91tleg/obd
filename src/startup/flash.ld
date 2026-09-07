/**
 * Linker script for STM32H7
 * 2048Kb FLASH, 1056Kb RAM
 */

ENTRY(reset_handler)

/* Top of main RAM (initial stack pointer) */
_estack = ORIGIN(RAM_D1) + LENGTH(RAM_D1);

_Min_Stack_Size = 0x400;

MEMORY
{
  FLASH   (rx)  : ORIGIN = 0x08000000, LENGTH = 2048K
  DTCMRAM (xrw) : ORIGIN = 0x20000000, LENGTH = 128K
  RAM_D1  (xrw) : ORIGIN = 0x24000000, LENGTH = 512K
  RAM_D2  (xrw) : ORIGIN = 0x30000000, LENGTH = 288K
  RAM_D3  (xrw) : ORIGIN = 0x38000000, LENGTH = 64K
  ITCMRAM (xrw) : ORIGIN = 0x00000000, LENGTH = 64K
}

SECTIONS
{
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector))
    . = ALIGN(4);
  } >FLASH

  .text :
  {
    . = ALIGN(4);

    *(.text)
    *(.text*)

    . = ALIGN(4);
    _etext = .;
  } >FLASH

  .rodata :
  {
    . = ALIGN(4);

    *(.rodata)
    *(.rodata*)

    . = ALIGN(4);
  } >FLASH

  /*
   * Initial values for .data are stored in FLASH.
   * Startup copies them to RAM.
   */
  _sidata = LOADADDR(.data);

  .data :
  {
    . = ALIGN(4);
    _sdata = .;

    *(.data)
    *(.data*)

    /* Functions explicitly placed in RAM */
    *(.RamFunc)
    *(.RamFunc*)

    . = ALIGN(4);
    _edata = .;
  } >RAM_D1 AT>FLASH

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
    __bss_end__ = _ebss;
  } >RAM_D1

  /*
   * Reserve minimum stack space.
   * Initial stack pointer is _estack.
   */
  ._user_stack :
  {
    . = ALIGN(8);

    . = . + _Min_Stack_Size;

    . = ALIGN(8);
  } >RAM_D1

  /* ARM build attributes */
  .ARM.attributes 0 :
  {
    *(.ARM.attributes)
  }
}
