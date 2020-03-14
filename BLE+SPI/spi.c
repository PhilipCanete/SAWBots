#include "nrfx_spim.h"
#include "app_util_platform.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "boards.h"
#include "app_error.h"
#include <string.h>
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#define NRFX_SPIM_SS_PIN   14
#define NRFX_SPIM_SCK_PIN  13
#define NRFX_SPIM_MOSI_PIN 12
#define NRFX_SPIM_MISO_PIN 11

#define SPI_INSTANCE  0                                           /**< SPI instance index. */
static const nrfx_spim_t spi = NRFX_SPIM_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */

static volatile bool spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */

//DDS Settings
const uint8_t DDSCHIPSELECT = 3; //Pin 14 on nRF52DK == pin 3 arduino - Find in variant.cpp
const uint8_t DDSSYNCIO = 8;     //Pin 19 == pin 8

//define values to send
const uint8_t DDSCONTROLREGISTER = 0x0;
#define DDSFTWREGISTER 0x04        //CHANGE FOR AD9913 to 0x03
static uint8_t tx_buf[] = {DDSFTWREGISTER};

const uint32_t LEFTFTW = 0x22222222; //10Mhz @ 75Mhz in
const uint32_t STRAIGHTFTW = 0x33333333; //15Mhz @ 75Mhz in
const uint32_t RIGHTFTW = 0x44444444; //20Mhz @ 75Mhz in
//

void spim_event_handler(nrfx_spim_evt_t const * p_event,
                       void *                  p_context)
{
    spi_xfer_done = true;
    NRF_LOG_INFO("Transfer completed.");
}

int main(void)
{
    bsp_board_init(BSP_INIT_LEDS);

    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    nrfx_spim_xfer_desc_t xfer_desc = NRFX_SPIM_SINGLE_XFER(tx_buf, 1, NULL, 0);

    nrfx_spim_config_t spi_config = NRFX_SPIM_DEFAULT_CONFIG;
    spi_config.frequency      = NRF_SPIM_FREQ_1M;
    spi_config.ss_pin         = NRFX_SPIM_SS_PIN;
    spi_config.miso_pin       = NRFX_SPIM_MISO_PIN;
    spi_config.mosi_pin       = NRFX_SPIM_MOSI_PIN;
    spi_config.sck_pin        = NRFX_SPIM_SCK_PIN;
    spi_config.ss_active_high = false;
    APP_ERROR_CHECK(nrfx_spim_init(&spi, &spi_config, spim_event_handler, NULL));

    NRF_LOG_INFO("NRFX SPIM example started.");

    while (1)
    {
        // Reset rx buffer and transfer done flag
        spi_xfer_done = false;

        APP_ERROR_CHECK(nrfx_spim_xfer(&spi, &xfer_desc, 0));

        while (!spi_xfer_done)
        {
            __WFE();
        }

        NRF_LOG_FLUSH();

        bsp_board_led_invert(BSP_BOARD_LED_0);
        nrf_delay_ms(200);
    }
}
