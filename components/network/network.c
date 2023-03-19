#include <string.h>
#include "network.h"
#include "private_network.h"
#include "sdkconfig.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_wifi_default.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "lwip/err.h"
#include "lwip/sys.h"

static const char *TAG = "Network:";

#if CONFIG_EXAMPLE_CONNECT_IPV6
/* types of ipv6 addresses to be displayed on ipv6 events */
const char *example_ipv6_addr_types_to_str[6] = {
    "ESP_IP6_ADDR_IS_UNKNOWN",
    "ESP_IP6_ADDR_IS_GLOBAL",
    "ESP_IP6_ADDR_IS_LINK_LOCAL",
    "ESP_IP6_ADDR_IS_SITE_LOCAL",
    "ESP_IP6_ADDR_IS_UNIQUE_LOCAL",
    "ESP_IP6_ADDR_IS_IPV4_MAPPED_IPV6"
};
#endif


bool is_our_netif(const char *prefix, esp_netif_t *netif)
{
    return strncmp(prefix, esp_netif_get_desc(netif), strlen(prefix) - 1) == 0;
}



esp_netif_t *get_netif_from_desc(const char *desc)
{
    esp_netif_t *netif = NULL;
    while ((netif = esp_netif_next(netif)) != NULL) {
        if (strcmp(esp_netif_get_desc(netif), desc) == 0) {
            return netif;
        }
    }
    return netif;
}



void print_all_netif_ips(const char *prefix)
{
    // iterate over active interfaces, and print out IPs of "our" netifs
    esp_netif_t *netif = NULL;
    esp_netif_ip_info_t ip;
    for (int i = 0; i < esp_netif_get_nr_of_ifs(); ++i) {
        netif = esp_netif_next(netif);
        if (is_our_netif(prefix, netif)) {
            ESP_LOGI(TAG, "Conexión activada: %s", esp_netif_get_desc(netif));
            ESP_ERROR_CHECK(esp_netif_get_ip_info(netif, &ip));

            ESP_LOGI(TAG, "- Dirección IPv4 : " IPSTR ",", IP2STR(&ip.ip));
#if CONFIG_EXAMPLE_CONNECT_IPV6
            esp_ip6_addr_t ip6[MAX_IP6_ADDRS_PER_NETIF];
            int ip6_addrs = esp_netif_get_all_ip6(netif, ip6);
            for (int j = 0; j < ip6_addrs; ++j) {
                esp_ip6_addr_type_t ipv6_type = esp_netif_ip6_get_addr_type(&(ip6[j]));
                ESP_LOGI(TAG, "- IPv6 address: " IPV6STR ", type: %s", IPV62STR(ip6[j]), example_ipv6_addr_types_to_str[ipv6_type]);
            }
#endif
        }
    }
}








esp_err_t network_connect(void)
{

#if CONFIG_ETH_ENABLE
    if (ethernet_init() != ESP_OK) {
        ESP_LOGE(TAG, "No se puede establecer conexión Ethernet");
        return ESP_FAIL;
    }
    ESP_ERROR_CHECK(esp_register_shutdown_handler(&ethernet_shutdown));
#endif// CREAR UNA TAREA ETHERNET


#if CONFIG_STA_ENABLE || CONFIG_AP_ENABLE  
    if (wifi_driver_init(WIFI_MODE_APSTA) != ESP_OK) {
        ESP_LOGE(TAG, "Error en la configuración WiFi");
        return ESP_FAIL;
    }
#endif
#if CONFIG_STA_ENABLE
    //ESP_ERROR_CHECK(esp_register_shutdown_handler(&wifi_shutdown_sta));
#endif


#if CONFIG_ETH_ENABLE
    print_all_netif_ips(ETH_NETIF_DESC);
#endif


#if CONFIG_STA_ENABLE
    print_all_netif_ips(STA_NETIF_DESC);
#endif

#if CONFIG_AP_ENABLE
    print_all_netif_ips(AP_NETIF_DESC);
#endif

    return ESP_OK;
}



esp_err_t network_disconnect(void)
{
#if CONFIG_ETH_ENABLE
    ethernet_shutdown();
    ESP_ERROR_CHECK(esp_unregister_shutdown_handler(&ethernet_shutdown));
#endif
#if CONFIG_STA_ENABLE
    wifi_shutdown_driver(WIFI_MODE_STA);
    ESP_ERROR_CHECK(esp_unregister_shutdown_handler(&wifi_shutdown_driver));
#endif
    return ESP_OK;
}
