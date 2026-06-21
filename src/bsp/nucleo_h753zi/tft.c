/**
 * @file    tft.c
 * @brief   BSP TFT display for Nucleo-H753ZI
 */

#include "bsp/nucleo_h753zi/tft.h"
#include "bsp/nucleo_h753zi/resources.h"
#include "hal/spi.h"
#include "hal/gpio.h"
#include "hal/rcc.h"

result_t bsp_tft_init( void )
{
    rcc_gpio_clk_enable( BSP_TFT_SCK_PORT  );
    rcc_gpio_clk_enable( BSP_TFT_MOSI_PORT );
    rcc_gpio_clk_enable( BSP_TFT_DC_PORT   );
    rcc_gpio_clk_enable( BSP_TFT_CS_PORT   );
    rcc_gpio_clk_enable( BSP_TFT_RST_PORT  );
    rcc_gpio_clk_enable( BSP_TFT_BL_PORT   );

    rcc_spi_clk_enable( BSP_TFT_SPI );

    gpio_set_af(    BSP_TFT_SCK_PORT,  BSP_TFT_SCK_PIN,  BSP_TFT_SCK_AF  );
    gpio_set_mode(  BSP_TFT_SCK_PORT,  BSP_TFT_SCK_PIN,  GPIO_MODE_AF    );
    gpio_set_speed( BSP_TFT_SCK_PORT,  BSP_TFT_SCK_PIN,  GPIO_SPEED_HIGH );
    gpio_set_pull(  BSP_TFT_SCK_PORT,  BSP_TFT_SCK_PIN,  GPIO_PULL_NONE  );

    gpio_set_af(    BSP_TFT_MOSI_PORT, BSP_TFT_MOSI_PIN, BSP_TFT_MOSI_AF );
    gpio_set_mode(  BSP_TFT_MOSI_PORT, BSP_TFT_MOSI_PIN, GPIO_MODE_AF    );
    gpio_set_speed( BSP_TFT_MOSI_PORT, BSP_TFT_MOSI_PIN, GPIO_SPEED_HIGH );
    gpio_set_pull(  BSP_TFT_MOSI_PORT, BSP_TFT_MOSI_PIN, GPIO_PULL_NONE  );

    gpio_set(       BSP_TFT_CS_PORT,  BSP_TFT_CS_PIN  );   /* deselect   */
    gpio_set_mode(  BSP_TFT_CS_PORT,  BSP_TFT_CS_PIN,  GPIO_MODE_OUTPUT );
    gpio_set_speed( BSP_TFT_CS_PORT,  BSP_TFT_CS_PIN,  GPIO_SPEED_HIGH  );
    gpio_set_pull(  BSP_TFT_CS_PORT,  BSP_TFT_CS_PIN,  GPIO_PULL_NONE   );

    gpio_set(       BSP_TFT_DC_PORT,  BSP_TFT_DC_PIN  );   /* data       */
    gpio_set_mode(  BSP_TFT_DC_PORT,  BSP_TFT_DC_PIN,  GPIO_MODE_OUTPUT );
    gpio_set_speed( BSP_TFT_DC_PORT,  BSP_TFT_DC_PIN,  GPIO_SPEED_HIGH  );
    gpio_set_pull(  BSP_TFT_DC_PORT,  BSP_TFT_DC_PIN,  GPIO_PULL_NONE   );

    gpio_set(       BSP_TFT_RST_PORT, BSP_TFT_RST_PIN );   /* released   */
    gpio_set_mode(  BSP_TFT_RST_PORT, BSP_TFT_RST_PIN, GPIO_MODE_OUTPUT );
    gpio_set_speed( BSP_TFT_RST_PORT, BSP_TFT_RST_PIN, GPIO_SPEED_LOW   );
    gpio_set_pull(  BSP_TFT_RST_PORT, BSP_TFT_RST_PIN, GPIO_PULL_NONE   );

    gpio_clear(     BSP_TFT_BL_PORT,  BSP_TFT_BL_PIN  );   /* off        */
    gpio_set_mode(  BSP_TFT_BL_PORT,  BSP_TFT_BL_PIN,  GPIO_MODE_OUTPUT );
    gpio_set_speed( BSP_TFT_BL_PORT,  BSP_TFT_BL_PIN,  GPIO_SPEED_LOW   );
    gpio_set_pull(  BSP_TFT_BL_PORT,  BSP_TFT_BL_PIN,  GPIO_PULL_NONE   );

    return spi_init( BSP_TFT_SPI,
                     SPI_MODE_3,
                     SPI_DIV_4,
                     SPI_FIRSTBIT_MSB );
}
