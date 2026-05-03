.syntax unified     /* ARM + Thumb2 in one file                        */
.cpu cortex-m7      /* target core                                     */
.fpu fpv5-d16       /* H753 FPU: double precision, 16 reg pairs        */
.thumb              /* emit Thumb2 instructions                        */

.global g_pfn_vectors
.global default_handler

/* these .word slots let the assembler resolve linker script symbols   */
.word _sidata       /* flash address where .data initializers live     */
.word _sdata        /* RAM start of .data                              */
.word _edata        /* RAM end   of .data                              */
.word _sbss         /* RAM start of .bss                               */
.word _ebss         /* RAM end   of .bss                               */

    .section .text.reset_handler
    .weak reset_handler
    .type reset_handler, %function
reset_handler:
    ldr   sp, =_estack          /* load stack pointer from linker symbol      */

    bl    exit_run0_mode        /* H7 boots in Run* (low-power) mode          */
                                /* must switch to normal Run before PLL touch */

    bl    system_init           /* configure PLLs, clocks, flash latency      */

    /* Copy .data */
    ldr   r0, =_sdata           /* r0 = dest start                            */
    ldr   r1, =_edata           /* r1 = dest end                              */
    ldr   r2, =_sidata          /* r2 = source in flash                       */
    movs  r3, #0                /* r3 = byte offset, start at 0               */
    b     loop_copy_data        /* jump to loop test first                    */
copy_data:
    ldr   r4, [r2, r3]          /* load word from flash[offset]               */
    str   r4, [r0, r3]          /* store word to   RAM[offset]                */
    adds  r3, r3, #4            /* advance offset by one word                 */
loop_copy_data:
    adds  r4, r0, r3            /* r4 = current dest address                  */
    cmp   r4, r1                /* past end of .data?                         */
    bcc   copy_data             /* no? keep copying (branch if carry clear)   */

    /* Zero .bss */
    ldr   r2, =_sbss            /* r2 = bss start                             */
    ldr   r4, =_ebss            /* r4 = bss end                               */
    movs  r3, #0                /* r3 = zero value to write                   */
    b     loop_zero_bss         /* jump to loop test first (bss may be empty) */
zero_bss:
    str   r3, [r2]              /* write zero to current bss address          */
    adds  r2, r2, #4            /* advance by one word                        */
loop_zero_bss:
    cmp   r2, r4                /* reached end of .bss?                       */
    bcc   zero_bss              /* no? keep zeroing                           */

    /* Enable FPU */
    ldr   r0, =0xE000ED88       /* r0 = address of CPACR                      */
    ldr   r1, [r0]              /* r1 = current CPACR value                   */
    orr   r1, r1, #(0xF << 20)  /* set CP10 and CP11 to full access            */
    str   r1, [r0]              /* write back                                 */
    dsb                         /* data sync barrier (ensure write completes) */
    isb                         /* instruction sync (flush pipeline)          */
                                /* pipeline must be flushed so the next fetch */
                                /* sees the FPU as enabled                    */

    bl    main                  /* jump to main                               */
    b     .                     /* main() should never return                 */
                                /* if it does, spin here rather than          */
                                /* executing garbage below the stack          */

    .size reset_handler, .-reset_handler

    .section .text.default_handler,"ax",%progbits
default_handler:
    b     .
    .size default_handler, .-default_handler

    .section .isr_vector,"a",%progbits
    .type g_pfn_vectors, %object
g_pfn_vectors:
    .word _estack
    .word reset_handler
    .word nmi_handler
    .word hard_fault_handler
    .word mem_manage_handler
    .word bus_fault_handler
    .word usage_fault_handler
    .word 0                         /* reserved */
    .word 0                         /* reserved */
    .word 0                         /* reserved */
    .word 0                         /* reserved */
    .word svc_handler
    .word debug_mon_handler
    .word 0                         /* reserved */
    .word pend_sv_handler
    .word sys_tick_handler
    /* External -- pad unreserved slots with default_handler,
       add a real handler name when you need it               */
    .word default_handler           /* WWDG                  */
    .word default_handler           /* PVD/AVD               */
    .word default_handler           /* TAMP/STAMP            */
    .word default_handler           /* RTC wakeup            */
    .word default_handler           /* FLASH                 */
    .word default_handler           /* RCC                   */
    .word default_handler           /* EXTI0                 */
    .word default_handler           /* EXTI1                 */
    .word default_handler           /* EXTI2                 */
    .word default_handler           /* EXTI3                 */
    .word default_handler           /* EXTI4                 */
    .word default_handler           /* DMA1 stream 0         */
    .word default_handler           /* DMA1 stream 1         */
    .word default_handler           /* DMA1 stream 2         */
    .word default_handler           /* DMA1 stream 3         */
    .word default_handler           /* DMA1 stream 4         */
    .word default_handler           /* DMA1 stream 5         */
    .word default_handler           /* DMA1 stream 6         */
    .word default_handler           /* ADC1/2/3              */
    .word default_handler           /* FDCAN1 IT0            */
    .word default_handler           /* FDCAN2 IT0            */
    .word default_handler           /* FDCAN1 IT1            */
    .word default_handler           /* FDCAN2 IT1            */
    .word default_handler           /* EXTI9..5              */
    .word default_handler           /* TIM1 BRK              */
    .word default_handler           /* TIM1 UP               */
    .word default_handler           /* TIM1 TRG/COM          */
    .word default_handler           /* TIM1 CC               */
    .word default_handler           /* TIM2                  */
    .word default_handler           /* TIM3                  */
    .word default_handler           /* TIM4                  */
    .word default_handler           /* I2C1 EV               */
    .word default_handler           /* I2C1 ER               */
    .word default_handler           /* I2C2 EV               */
    .word default_handler           /* I2C2 ER               */
    .word default_handler           /* SPI1                  */
    .word default_handler           /* SPI2                  */
    .word usart1_irq_handler        /* USART1                */
    .word usart2_irq_handler        /* USART2                */
    .word usart3_irq_handler        /* USART3                */
    .word default_handler           /* EXTI15..10            */
    .word default_handler           /* RTC alarm             */
    .word 0                         /* reserved              */
    .word default_handler           /* TIM8 BRK / TIM12      */
    .word default_handler           /* TIM8 UP  / TIM13      */
    .word default_handler           /* TIM8 TRG / TIM14      */
    .word default_handler           /* TIM8 CC               */
    .word default_handler           /* DMA1 stream 7         */
    .word default_handler           /* FMC                   */
    .word default_handler           /* SDMMC1                */
    .word default_handler           /* TIM5                  */
    .word default_handler           /* SPI3                  */
    .word uart4_irq_handler         /* UART4                 */
    .word uart5_irq_handler         /* UART5                 */
    .word default_handler           /* TIM6 / DAC            */
    .word default_handler           /* TIM7                  */
    .word default_handler           /* DMA2 stream 0         */
    .word default_handler           /* DMA2 stream 1         */
    .word default_handler           /* DMA2 stream 2         */
    .word default_handler           /* DMA2 stream 3         */
    .word default_handler           /* DMA2 stream 4         */
    .word default_handler           /* ETH                   */
    .word default_handler           /* ETH wakeup            */
    .word default_handler           /* FDCAN cal             */
    .word 0                         /* reserved              */
    .word 0                         /* reserved              */
    .word 0                         /* reserved              */
    .word 0                         /* reserved              */
    .word default_handler           /* DMA2 stream 5         */
    .word default_handler           /* DMA2 stream 6         */
    .word default_handler           /* DMA2 stream 7         */
    .word usart6_irq_handler        /* USART6                */
    .word default_handler           /* I2C3 EV               */
    .word default_handler           /* I2C3 ER               */
    .word default_handler           /* OTG HS EP1 OUT        */
    .word default_handler           /* OTG HS EP1 IN         */
    .word default_handler           /* OTG HS wakeup         */
    .word default_handler           /* OTG HS                */
    .word default_handler           /* DCMI                  */
    .word default_handler           /* CRYP                  */
    .word default_handler           /* HASH/RNG              */
    .word default_handler           /* FPU                   */
    .word uart7_irq_handler         /* UART7                 */
    .word uart8_irq_handler         /* UART8                 */
    .word default_handler           /* SPI4                  */
    .word default_handler           /* SPI5                  */
    .word default_handler           /* SPI6                  */
    .word default_handler           /* SAI1                  */
    .word default_handler           /* LTDC                  */
    .word default_handler           /* LTDC ER               */
    .word default_handler           /* DMA2D                 */
    .word default_handler           /* SAI2                  */
    .word default_handler           /* QUADSPI               */
    .word default_handler           /* LPTIM1                */
    .word default_handler           /* CEC                   */
    .word default_handler           /* I2C4 EV               */
    .word default_handler           /* I2C4 ER               */
    .word default_handler           /* SPDIF RX              */
    .word default_handler           /* OTG FS EP1 OUT        */
    .word default_handler           /* OTG FS EP1 IN         */
    .word default_handler           /* OTG FS wakeup         */
    .word default_handler           /* OTG FS                */
    .word default_handler           /* DMAMUX1 OVR           */
    .word default_handler           /* HRTIM1 master         */
    .word default_handler           /* HRTIM1 TIMA           */
    .word default_handler           /* HRTIM1 TIMB           */
    .word default_handler           /* HRTIM1 TIMC           */
    .word default_handler           /* HRTIM1 TIMD           */
    .word default_handler           /* HRTIM1 TIME           */
    .word default_handler           /* HRTIM1 FLT            */
    .word default_handler           /* DFSDM1 FLT0           */
    .word default_handler           /* DFSDM1 FLT1           */
    .word default_handler           /* DFSDM1 FLT2           */
    .word default_handler           /* DFSDM1 FLT3           */
    .word default_handler           /* SAI3                  */
    .word default_handler           /* SWPMI1                */
    .word default_handler           /* TIM15                 */
    .word default_handler           /* TIM16                 */
    .word default_handler           /* TIM17                 */
    .word default_handler           /* MDIOS wakeup          */
    .word default_handler           /* MDIOS                 */
    .word default_handler           /* JPEG                  */
    .word default_handler           /* MDMA                  */
    .word 0                         /* reserved              */
    .word default_handler           /* SDMMC2                */
    .word default_handler           /* HSEM1                 */
    .word 0                         /* reserved              */
    .word default_handler           /* ADC3                  */
    .word default_handler           /* DMAMUX2 OVR           */
    .word default_handler           /* BDMA ch0              */
    .word default_handler           /* BDMA ch1              */
    .word default_handler           /* BDMA ch2              */
    .word default_handler           /* BDMA ch3              */
    .word default_handler           /* BDMA ch4              */
    .word default_handler           /* BDMA ch5              */
    .word default_handler           /* BDMA ch6              */
    .word default_handler           /* BDMA ch7              */
    .word default_handler           /* COMP1                 */
    .word default_handler           /* LPTIM2                */
    .word default_handler           /* LPTIM3                */
    .word default_handler           /* LPTIM4                */
    .word default_handler           /* LPTIM5                */
    .word default_handler           /* LPUART1               */
    .word 0                         /* reserved              */
    .word default_handler           /* CRS                   */
    .word default_handler           /* ECC                   */
    .word default_handler           /* SAI4                  */
    .word 0                         /* reserved              */
    .word 0                         /* reserved              */
    .word default_handler           /* wakeup pin            */
    .size g_pfn_vectors, .-g_pfn_vectors

    /* Core exception weak aliases */
    .weak nmi_handler
    .thumb_set nmi_handler, default_handler
    .weak hard_fault_handler
    .thumb_set hard_fault_handler, default_handler
    .weak mem_manage_handler
    .thumb_set mem_manage_handler, default_handler
    .weak bus_fault_handler
    .thumb_set bus_fault_handler, default_handler
    .weak usage_fault_handler
    .thumb_set usage_fault_handler, default_handler
    .weak svc_handler
    .thumb_set svc_handler, default_handler
    .weak debug_mon_handler
    .thumb_set debug_mon_handler, default_handler
    .weak pend_sv_handler
    .thumb_set pend_sv_handler, default_handler
    .weak sys_tick_handler
    .thumb_set sys_tick_handler, default_handler

    /* Weak-alias we need */
    /* Add an entry here when we promote a default_handler slot. */
    .weak usart1_irq_handler
    .thumb_set usart1_irq_handler, default_handler
    .weak usart2_irq_handler
    .thumb_set usart2_irq_handler, default_handler
    .weak usart3_irq_handler
    .thumb_set usart3_irq_handler, default_handler
    .weak usart6_irq_handler
    .thumb_set usart6_irq_handler, default_handler
    .weak uart4_irq_handler
    .thumb_set uart4_irq_handler, default_handler
    .weak uart5_irq_handler
    .thumb_set uart5_irq_handler, default_handler
    .weak uart7_irq_handler
    .thumb_set uart7_irq_handler, default_handler
    .weak uart8_irq_handler
    .thumb_set uart8_irq_handler, default_handler
