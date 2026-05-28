/**
 * @file    resources.h
 * @brief   STM32H753ZI Nucleo board resource definitions
 */

#ifndef BSP_NUCLEO_H753ZI_RESOURCES_H
#define BSP_NUCLEO_H753ZI_RESOURCES_H

/* Clock tree */
#define BSP_PLL1_M              ( 2U )    /* HSE 8MHz / 2 = 4MHz ref     */
#define BSP_PLL1_N              ( 240U )  /* 4MHz * 240 = 960MHz VCO     */
#define BSP_PLL1_P              ( 2U )    /* 960MHz / 2 = 480MHz SYSCLK  */
#define BSP_PLL1_Q              ( 20U )   /* 960MHz / 20 = 48MHz USB     */
#define BSP_PLL1_R              ( 2U )    /* 960MHz / 2 = 480MHz unused  */
#define BSP_FLASH_LATENCY       ( 4U )    /* wait states for 480MHz VOS1 */
#define BSP_SYSCLK_HZ           ( 480000000UL )

/* LED */
#define BSP_LED_GREEN_PORT      ( GPIOB )
#define BSP_LED_GREEN_PIN       ( 0U )

/* User button */
#define BSP_BTN_USER_PORT       ( GPIOC )
#define BSP_BTN_USER_PIN        ( 13U )
#define BSP_BTN_IRQn            ( EXTI15_10_IRQn )

/* CAN (FDCAN1 — PD0/PD1, AF9) */
#define BSP_CAN                 ( FDCAN1 )
#define BSP_CAN_TX_PORT         ( GPIOD )
#define BSP_CAN_TX_PIN          ( 1U )
#define BSP_CAN_TX_AF           ( 9U )
#define BSP_CAN_RX_PORT         ( GPIOD )
#define BSP_CAN_RX_PIN          ( 0U )
#define BSP_CAN_RX_AF           ( 9U )

/* Debug UART (ST-Link virtual COM) */
#define BSP_DEBUG_UART          ( USART3 )
#define BSP_DEBUG_TX_PORT       ( GPIOD )
#define BSP_DEBUG_TX_PIN        ( 8U )
#define BSP_DEBUG_TX_AF         ( 7U )
#define BSP_DEBUG_RX_PORT       ( GPIOD )
#define BSP_DEBUG_RX_PIN        ( 9U )
#define BSP_DEBUG_RX_AF         ( 7U )
#define BSP_DEBUG_IRQn          ( USART3_IRQn )
#define BSP_UART_CH_DEBUG       ( 0U )

/* OBD UART */
#define BSP_OBD_UART            ( UART4 )
#define BSP_OBD_TX_PORT         ( GPIOA )
#define BSP_OBD_TX_PIN          ( 0U )
#define BSP_OBD_TX_AF           ( 8U )
#define BSP_OBD_RX_PORT         ( GPIOA )
#define BSP_OBD_RX_PIN          ( 1U )
#define BSP_OBD_RX_AF           ( 8U )
#define BSP_OBD_IRQn            ( UART4_IRQn )
#define BSP_UART_CH_OBD         ( 1U )

/* LCD (I2C) */
#define BSP_LCD_I2C             ( I2C1 )
#define BSP_LCD_SCL_PORT        ( GPIOB )
#define BSP_LCD_SCL_PIN         ( 8U )
#define BSP_LCD_SCL_AF          ( 4U )
#define BSP_LCD_SDA_PORT        ( GPIOB )
#define BSP_LCD_SDA_PIN         ( 9U )
#define BSP_LCD_SDA_AF          ( 4U )

/* TFT display (SPI) */
#define BSP_TFT_SPI             ( SPI2 )
#define BSP_TFT_SCK_PORT        ( GPIOB )
#define BSP_TFT_SCK_PIN         ( 13U )
#define BSP_TFT_SCK_AF          ( 5U )
#define BSP_TFT_MOSI_PORT       ( GPIOB )
#define BSP_TFT_MOSI_PIN        ( 15U )
#define BSP_TFT_MOSI_AF         ( 5U )
#define BSP_TFT_DC_PORT         ( GPIOC )
#define BSP_TFT_DC_PIN          ( 6U )
#define BSP_TFT_CS_PORT         ( GPIOC )
#define BSP_TFT_CS_PIN          ( 7U )
#define BSP_TFT_RST_PORT        ( GPIOA )
#define BSP_TFT_RST_PIN         ( 8U )
#define BSP_TFT_BL_PORT         ( GPIOA )
#define BSP_TFT_BL_PIN          ( 9U )

#endif /* BSP_NUCLEO_H753ZI_RESOURCES_H */
