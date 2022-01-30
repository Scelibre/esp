#pragma once

#include <esp_wifi.h>

#define WIFI_AUTHMODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#define WIFI_AP_MAX_CONNECTION 4

extern struct wifi_config {
  int mode; /* wifi_mode_t */
  int auth_mode; /* wifi_auth_mode_t */
  uint16_t channel;
  char ssid[32];
  char password[64];
  char hostname[32];
  char ip[16];
  char netmask[15];
  char gw[15];
} wifi_config;

int init_wifi_events();
int init_wifi_system();
int init_wifi_timer();

int config_wifi(const struct wifi_config *config);

int start_wifi();
int stop_wifi();

void enable_wifi_reconnect();
void disable_wifi_reconnect();
void start_wifi_reconnect();
void cancel_wifi_reconnect();

int wifi_scan(const wifi_scan_config_t *scan_config, int (*cb)(wifi_ap_record_t *ap, void *ctx), void *ctx);
int wifi_listen(const wifi_ap_config_t *config);
int wifi_connect(const wifi_sta_config_t *config);
int wifi_disconnect();
int wifi_reconnect();
int wifi_disable();