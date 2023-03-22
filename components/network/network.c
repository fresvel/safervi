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

#include "flash.h"

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



esp_err_t network_set_flash_default(){


    flash_net_ena_t flash_net_ena={
        .eth =true,
        .sta =true,
        .ap =true,
    };

    size_t req_size = sizeof(flash_net_ena_t);
    printf("req_size = %d\n", req_size);

    ESP_LOGW(TAG, "Escritura de enable: %s",esp_err_to_name(flash__network_write_label(&flash_net_ena, "ena", req_size)));
    


        flash_sta_t flash_sta={
            .ssid=CONFIG_STA_WIFI_SSID,      
            .password=CONFIG_STA_WIFI_PASSWORD,
            .channel=1,
            .dhcp=NET_DHCP_CLIENT,
            .ip_info.ip={192,168,0,7},
            .ip_info.netmask={255,255,255,0},
            .ip_info.gw={192,168,0,1},
            .dns[0]={192,168,0,1},
            .dns[1]={0,0,0,0},
            .dns[2]={0,0,0,0},
            .ip6_addr.addr={0XFE800000,0X0, 0x0, 0x7},
            .ip6_addr.zone=0,//verificar si es necesario
            .ip6_type=ESP_IP6_ADDR_IS_LINK_LOCAL,
            .dns6[0].addr={0X0,0X0, 0x0, 0x0},
            .dns6[1].addr={0X0,0X0, 0x0, 0x0},
            .dns6[2].addr={0X0,0X0, 0x0, 0x0},
            .max_retry=10,
        };

    req_size = sizeof(flash_sta_t);

    printf("req_size = %d\n", req_size);
    
    ESP_LOGW(TAG, "Escritura de STA: %s", esp_err_to_name(flash__network_write_label(&flash_sta, "sta", req_size)));

        flash_ap_t flash_ap={
            .ssid=CONFIG_AP_WIFI_SSID,      
            .password=CONFIG_AP_WIFI_PASSWORD,
            .channel=CONFIG_AP_WIFI_CHANNEL,
            .ip_info.ip={192,168,2,7},
            .ip_info.netmask={255,255,255,0},
            .ip_info.gw={192,168,2,1},
            .dns={192,168,2,1},
            .ip6_addr.addr={0XFE800000,0X2, 0x0, 0x7},
            .ip6_addr.zone=1,//verificar si es necesario
            .ip6_type=ESP_IP6_ADDR_IS_LINK_LOCAL,
            .dns6.addr={0X0,0X0, 0x0, 0x0},
            .max_sta=10,
            .auth_mode=AP_WIFI_AUTH_MODE,
        };

    
    req_size = sizeof(flash_ap_t);
    printf("req_size = %d\n", req_size);
    
    ESP_LOGW(TAG, "Escritura de STA: %s", esp_err_to_name(flash__network_write_label(&flash_ap, "ap", req_size)));
    


        flash_eth_t flash_eth={
            .ip_info.ip={192,168,1,7},
            .ip_info.netmask={255,255,255,0},
            .ip_info.gw={192,168,1,1},
            .dns[0]={192,168,1,1},
            .dns[1]={0,0,0,0},
            .dns[2]={0,0,0,0},
            .dhcp=NET_DHCP_CLIENT,
            .ip6_addr.addr={0XFE800000,0X1, 0x0, 0x7},
            .ip6_addr.zone=1,//verificar si es necesario
            .ip6_type=ESP_IP6_ADDR_IS_LINK_LOCAL,
            .dns6[0].addr={0X0,0X0, 0x0, 0x0},
            .dns6[1].addr={0X0,0X0, 0x0, 0x0},
            .dns6[2].addr={0X0,0X0, 0x0, 0x0},
        };

    
    req_size = sizeof(flash_eth_t);
    printf("req_size = %d\n", req_size);
    
    ESP_LOGW(TAG, "Escritura de STA: %s", esp_err_to_name(flash__network_write_label(&flash_eth, "eth", req_size)));
    


        flash_ast_t flash_ast={
            .ssid="",
            .password="",
            .max_retry=0,
        };
    req_size =sizeof(flash_ast_t);
    printf("req_size = %d\n", req_size);

    ESP_LOGW(TAG, "Escritura de AST: %s", esp_err_to_name(flash__network_write_label(&flash_ast, "ast", req_size)));
    return ESP_OK;

}








esp_err_t network_connect(void)
{

    ESP_LOGI(TAG, "Estado de init%s", esp_err_to_name(flash_network_init()));

    flash_net_ena_t* net_ena;

    size_t req_size = sizeof(flash_net_ena_t);
    net_ena = malloc(req_size);
    esp_err_t ret = flash__network_get_label(net_ena, "ena", req_size);
    
    if (ret == ESP_OK) {
        printf("eth:%d  sta: %d ap: %d\n", net_ena->eth, net_ena->sta, net_ena->ap);
    
    
    }else if(ret == ESP_ERR_NVS_NOT_FOUND){
        ESP_ERROR_CHECK(network_set_flash_default());
        ESP_LOGW(TAG, "Memoria grabada, reiniciando el dispositivo");
        ESP_ERROR_CHECK(flash__network_get_label(net_ena, "ena", req_size));
        ESP_LOGI(TAG,"eth:%d  sta: %d ap: %d\n", net_ena->eth, net_ena->sta, net_ena->ap);
    }else {
        free(net_ena);
        return ret;
    }
    ESP_LOGI(TAG, "Estado de Enable %s",esp_err_to_name(ret));
    free(net_ena);








#if CONFIG_ETH_ENABLE


xTaskCreate(ethernet_init,"Ethernet",2048,NULL,10,NULL);

/*
    if (ethernet_init() != ESP_OK) {
        ESP_LOGE(TAG, "No se puede establecer conexión Ethernet");
        return ESP_FAIL;
    }
    ESP_ERROR_CHECK(esp_register_shutdown_handler(&ethernet_shutdown));

*/
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
