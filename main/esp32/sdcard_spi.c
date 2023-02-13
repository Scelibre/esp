#include "../sdcard_spi.h"

#include <logging.h>

#if CONFIG_SDCARD_SPI_HOST
  #include <driver/spi_master.h>
  #include <driver/sdspi_host.h>
  #include <driver/sdmmc_types.h>

  // TODO: shared? conflict with leds_spi?
  #define SDCARD_SPI_HOST SPI2_HOST
  #define SDCARD_SPI_DMA_CH SPI_DMA_CH_AUTO
  #define SDCARD_SPI_MOSI_IO_NUM 15
  #define SDCARD_SPI_MISO_IO_NUM 2
  #define SDCARD_SPI_SCLK_IO_NUM 14
  #define SDCARD_SPI_CS_IO_NUM 13
  #define SDCARD_SPI_MAX_TRANSFER_SZ 4000 // XXX: whence? One DMA buffer?
  #define SDCARD_SPI_MAX_FREQ_KHZ 10000 // 10MHz for crappy breadboard testing

  sdmmc_host_t sdcard_spi_host = SDSPI_HOST_DEFAULT();

  int init_sdcard_spi(sdmmc_host_t **hostp)
  {
    spi_bus_config_t bus_config = {
      .mosi_io_num      = SDCARD_SPI_MOSI_IO_NUM,
      .miso_io_num      = SDCARD_SPI_MISO_IO_NUM,
      .sclk_io_num      = SDCARD_SPI_SCLK_IO_NUM,
      .quadwp_io_num    = -1,
      .quadhd_io_num    = -1,
      .max_transfer_sz  = SDCARD_SPI_MAX_TRANSFER_SZ,
    };
    sdspi_device_config_t dev_config = {
      .host_id          = SDCARD_SPI_HOST,
      .gpio_cs          = SDCARD_SPI_CS_IO_NUM,
      .gpio_cd          = SDSPI_SLOT_NO_CD,
      .gpio_wp          = SDSPI_SLOT_NO_WP,
      .gpio_int         = SDSPI_SLOT_NO_INT,
    };
    sdspi_dev_handle_t dev;
    esp_err_t err;

    if ((err = spi_bus_initialize(SDCARD_SPI_HOST, &bus_config, SDCARD_SPI_DMA_CH))) {
      LOG_ERROR("spi_bus_initialize(%d): %s", SDCARD_SPI_HOST, esp_err_to_name(err));
      return -1;
    }

    if ((err = sdspi_host_init())) {
      LOG_ERROR("sdspi_host_init: %s", esp_err_to_name(err));
      return -1;
    }

    if ((err = sdspi_host_init_device(&dev_config, &dev))) {
      LOG_ERROR("sdspi_host_init_device: %s", esp_err_to_name(err));
      return -1;
    } else {
      sdcard_spi_host.slot = dev;
      sdcard_spi_host.max_freq_khz = SDCARD_SPI_MAX_FREQ_KHZ;
    }

    LOG_INFO("using spi_host=%d slot=%d max_freq_khz=%d",
      SDCARD_SPI_HOST,
      sdcard_spi_host.slot,
      sdcard_spi_host.max_freq_khz
    );

    *hostp = &sdcard_spi_host;

    return 0;
  }
#endif
