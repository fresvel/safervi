#pragma once
#include "esp_netif.h"
#include "esp_wifi.h"
#include <nvs.h>
#include <nvs_flash.h>
#include <string.h>

typedef struct{
    uint8_t ssid[32];      
    uint8_t password[64];  
    uint8_t channel;
    bool dhcp;
    esp_ip4_addr_t ip_add;
    esp_ip4_addr_t ip_net;
    esp_ip4_addr_t ip_gw;
    esp_ip4_addr_t dns_add;
    esp_ip4_addr_t dns_alt;
    esp_ip6_addr_t ip6_addr;
    esp_ip6_addr_t ip6_gw;
    esp_ip6_addr_type_t ip6_type;
    esp_ip6_addr_t dns6_add;
    esp_ip6_addr_t dns6_alt;
    uint32_t max_retry;           
} flash_sta_t;

typedef struct{
    uint8_t ssid[32];      
    uint8_t password[64];  
    uint32_t max_retry;           
} flash_ast_t;


typedef struct{
    uint8_t ssid[32];      
    uint8_t password[64];  
    uint8_t channel;
    esp_ip4_addr_t ip_add;
    esp_ip4_addr_t ip_net;
    esp_ip4_addr_t ip_gw;
    esp_ip4_addr_t dns_add;
    esp_ip4_addr_t dns_alt;
    esp_ip6_addr_t ip6_addr;
    esp_ip6_addr_t ip6_gw;
    esp_ip6_addr_type_t ip6_type;
    esp_ip6_addr_t dns6_add;
    esp_ip6_addr_t dns6_alt;
    uint8_t max_sta;
    wifi_auth_mode_t auth_mode;
} flash_ap_t;


typedef struct{
    esp_ip4_addr_t ip_add;
    esp_ip4_addr_t ip_net;
    esp_ip4_addr_t ip_gw;
    esp_ip4_addr_t dns_add;
    esp_ip4_addr_t dns_alt;
    esp_ip6_addr_t ip6_addr;
    esp_ip6_addr_t ip6_gw;
    esp_ip6_addr_type_t ip6_type;
    esp_ip6_addr_t dns6_add;
    esp_ip6_addr_t dns6_alt;
} flash_eth_t;


typedef struct{
    bool sta;
    bool ap;
    bool eth;
    bool ast;
    bool mqtt;
    bool ws_c;
    bool ws_s;
}flash_net_ena_t;


esp_err_t flash__network_write_label(void* flash_value, const char* label, size_t req_size);

esp_err_t flash__network_get_label(void* flash_buff, const char* label, size_t req_size);

esp_err_t flash_network_init();

void show_file(void);
