#ifndef __SYSTEM_H__
#define __SYSTEM_H__

#include <stddef.h>
#include <esp_system.h>
#include <esp_ota_ops.h>

struct system_image_info {
  void *iram_start, *dram_start, *flash_start;
  void *iram_end, *dram_end, *flash_end;
  size_t iram_usage, dram_usage, flash_usage;
  size_t iram_size, dram_size;

  void *iram_heap_start, *dram_heap_start;
  size_t iram_heap_size, dram_heap_size;
};

/*
 * Static info from system image
 */
struct system_info {
    esp_chip_info_t esp_chip_info;
    struct system_image_info image_info;

    const char* esp_idf_version;
    const esp_app_desc_t *esp_app_desc;
    size_t spi_flash_chip_size;
};

/*
 * Dynamic info from system state
 */
struct system_status {
  esp_reset_reason_t reset_reason;
  int uptime; // us
  int cpu_frequency; // gz

  size_t total_heap_size, free_heap_size, minimum_free_heap_size;
};

size_t system_get_total_heap_size();

void system_image_info(struct system_image_info *info);
void system_info_get(struct system_info *info);
void system_status_get(struct system_status *status);

/* Return string for reset reason */
const char *esp_reset_reason_str(esp_reset_reason_t reason);

#endif
