/**
 * @file    resources.h
 * @brief   STM32H753ZI Nucleo board resource definitions
 */

#ifndef BSP_NUCLEO_H753ZI_RESOURCES_H
#define BSP_NUCLEO_H753ZI_RESOURCES_H

/* Clock tree */
#define BSP_PLL1_M            ( 2U )    /* HSE 8MHz / 2 = 4MHz ref       */
#define BSP_PLL1_N            ( 240U )  /* 4MHz * 240 = 960MHz VCO       */
#define BSP_PLL1_P            ( 2U )    /* 960MHz / 2 = 480MHz SYSCLK    */
#define BSP_PLL1_Q            ( 20U )   /* 960MHz / 20 = 48MHz USB       */
#define BSP_PLL1_R            ( 2U )    /* 960MHz / 2 = 480MHz unused    */
#define BSP_FLASH_LATENCY     ( 4U )    /* wait states for 480MHz VOS1   */
#define BSP_SYSCLK_HZ         ( 480000000UL )

/* LED */
#define BSP_LED_GREEN_PORT    ( GPIOB )
#define BSP_LED_GREEN_PIN     ( 0U )

/* User button */
#define BSP_BTN_USER_PORT     ( GPIOC )
#define BSP_BTN_USER_PIN      ( 13U )

/* CAN (FDCAN1 — PD0/PD1, AF9) */
#define BSP_CAN               ( FDCAN1 )
#define BSP_CAN_TX_PORT       ( GPIOD )
#define BSP_CAN_TX_PIN        ( 1U )
#define BSP_CAN_TX_AF         ( 9U )
#define BSP_CAN_RX_PORT       ( GPIOD )
#define BSP_CAN_RX_PIN        ( 0U )
#define BSP_CAN_RX_AF         ( 9U )
#define BSP_CAN_TIMING        ( 0x001C0013U )  /* 500kbps @ 80MHz */

/* Debug UART (ST-Link virtual COM) */
#define BSP_DEBUG_RX_BUF_SIZE   ( 256U )
#define BSP_DEBUG_TX_BUF_SIZE   ( 256U )
#define BSP_DEBUG_UART          ( USART3 )
#define BSP_DEBUG_BAUD          ( 115200U )
#define BSP_DEBUG_TX_PORT       ( GPIOD )
#define BSP_DEBUG_TX_PIN        ( 8U )
#define BSP_DEBUG_TX_AF         ( 7U )
#define BSP_DEBUG_RX_PORT       ( GPIOD )
#define BSP_DEBUG_RX_PIN        ( 9U )
#define BSP_DEBUG_RX_AF         ( 7U )
#define BSP_DEBUG_IRQn          ( USART3_IRQn )
#define BSP_DEBUG_IRQ_PRIORITY  ( 6U )
#define BSP_UART_CH_DEBUG       ( 0U )

/* OBD UART */
#define BSP_OBD_RX_BUF_SIZE     ( 64U )
#define BSP_OBD_TX_BUF_SIZE     ( 64U )
#define BSP_OBD_UART            ( UART4 )
#define BSP_OBD_BAUD            ( 10400U )
#define BSP_OBD_TX_PORT         ( GPIOA )
#define BSP_OBD_TX_PIN          ( 0U )
#define BSP_OBD_TX_AF           ( 8U )
#define BSP_OBD_RX_PORT         ( GPIOA )
#define BSP_OBD_RX_PIN          ( 1U )
#define BSP_OBD_RX_AF           ( 8U )
#define BSP_OBD_IRQn            ( UART4_IRQn )
#define BSP_OBD_IRQ_PRIORITY    ( 5U )   /* higher priority than debug   */
#define BSP_UART_CH_OBD         ( 1U )

/* LCD (I2C) */
#define BSP_LCD_I2C           ( I2C1 )
#define BSP_LCD_SCL_PORT      ( GPIOB )
#define BSP_LCD_SCL_PIN       ( 8U )
#define BSP_LCD_SCL_AF        ( 4U )
#define BSP_LCD_SDA_PORT      ( GPIOB )
#define BSP_LCD_SDA_PIN       ( 9U )
#define BSP_LCD_SDA_AF        ( 4U )
#define BSP_LCD_I2C_ADDR      ( 0x27U )
#define BSP_LCD_I2C_SPEED_HZ  ( 100000U )
#define BSP_LCD_I2C_TIMING    ( 0x00C0EAFFU )

#endif /* BSP_NUCLEO_H753ZI_RESOURCES_H */
