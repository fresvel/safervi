/* Common functions to establish Wi-Fi or Ethernet connection.

   This code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
 */

#pragma once
#include "sdkconfig.h"
#include "esp_err.h"
#include "esp_netif.h"
#if CONFIG_ETH_ENABLE
#include "esp_eth.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if CONFIG_STA_ENABLE
#define STA_NETIF_DESC "Wi-Fi STA"
#endif

#if CONFIG_AP_ENABLE
#define AP_NETIF_DESC "Wi-Fi AP"
#endif

#if CONFIG_ETH_ENABLE
#define ETH_NETIF_DESC "Ethernet"
#endif




#if CONFIG_AP_WIFI_AUTH_OPEN
#define AP_WIFI_AUTH_MODE WIFI_AUTH_OPEN
#elif CONFIG_AP_WIFI_AUTH_WPA_PSK
#define AP_WIFI_AUTH_MODE WIFI_AUTH_WPA_PSK
#elif CONFIG_AP_WIFI_AUTH_WPA2_PSK
#define AP_WIFI_AUTH_MODE WIFI_AUTH_WPA2_PSK
#elif CONFIG_AP_WIFI_AUTH_WPA_WPA2_PSK
#define AP_WIFI_AUTH_MODE WIFI_AUTH_WPA_WPA2_PSK
#endif







esp_err_t network_connect(void);
esp_err_t network_disconnect(void);
esp_netif_t *get_netif_from_desc(const char *desc);

#if CONFIG_ETH_ENABLE
esp_eth_handle_t get_eth_handle(void);
#endif 

#ifdef __cplusplus
}
#endif
