#pragma once
#include "esp_netif.h"
#include "esp_wifi.h"
#include <nvs.h>
#include <nvs_flash.h>
#include <string.h>

typedef uint8_t dhcp_type_t;

#define NET_DHCP_NULL (0)
#define NET_DHCP_CLIENT (1)
#define NET_DHCP_SERVER (2)

typedef struct{
    uint8_t ssid[32];      
    uint8_t password[64];  
    uint8_t channel;
    dhcp_type_t dhcp;    
    esp_netif_ip_info_t ip_info;
    esp_ip4_addr_t dns[3];
    esp_ip6_addr_t ip6_addr;
    esp_ip6_addr_type_t ip6_type;
    esp_ip6_addr_t dns6[3];
    uint8_t max_retry;           
} flash_sta_t;


typedef struct{
    uint8_t ssid[32];      
    uint8_t password[64];  
    uint8_t channel;
    esp_netif_ip_info_t ip_info;
    esp_ip4_addr_t dns;
    esp_ip6_addr_t ip6_addr;
    esp_ip6_addr_type_t ip6_type;
    esp_ip6_addr_t dns6;
    wifi_auth_mode_t auth_mode;
    uint8_t max_sta;
} flash_ap_t;



typedef struct{
    esp_netif_ip_info_t ip_info;
    esp_ip4_addr_t dns[3];
    dhcp_type_t dhcp;
    esp_ip6_addr_t ip6_addr;
    esp_ip6_addr_type_t ip6_type;
    esp_ip6_addr_t dns6[3];
} flash_eth_t;

typedef struct{
    uint8_t ssid[32];      
    uint8_t password[64];  
    uint8_t max_retry;           
} flash_ast_t;


typedef struct{
    bool sta;
    bool ap;
    bool eth;
    bool ast;
}flash_net_ena_t;

typedef struct{
    flash_ap_t ap;
    flash_sta_t sta;
    wifi_mode_t mode;
}flash_wifi_t;


//Evaluar la posibilidad de usar o no, se crea un acceso global a las variables por defecto
typedef struct{
    flash_net_ena_t ena;
    flash_sta_t sta;
    flash_eth_t eth;
    flash_ap_t ap;
    flash_ast_t ast;
}flash_net_def_t;


typedef struct{
    bool mqtt;
    bool ws_c;
    bool ws_s;
}flash_prot_ena_t;











esp_err_t flash__network_write_label(void* flash_value, const char* label, size_t req_size);

esp_err_t flash__network_get_label(void* flash_buff, const char* label, size_t req_size);

esp_err_t flash_network_init();

void show_file(void);
