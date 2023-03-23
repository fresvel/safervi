/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <string.h>
#include "network.h"
#include "private_network.h"
#include "esp_event.h"
#include "esp_eth.h"
#if CONFIG_ETH_USE_SPI_ETHERNET
#include "driver/spi_master.h"
#endif // CONFIG_ETH_USE_SPI_ETHERNET
#include <esp_log.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_private/wifi.h"
#include "esp_wifi.h"


static const char *TAG = "Ethernet";
static SemaphoreHandle_t s_semph_get_ip_addrs = NULL;
#if CONFIG_EXAMPLE_CONNECT_IPV6
static SemaphoreHandle_t s_semph_get_ip6_addrs = NULL;
#endif


static void eth_stop(void);
static esp_eth_handle_t s_eth_handle = NULL;
static esp_eth_mac_t *s_mac = NULL;
static esp_eth_phy_t *s_phy = NULL;
static esp_eth_netif_glue_handle_t s_eth_glue = NULL;




/** Event handler for Ethernet events */

static void eth_on_got_ip(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    if (!is_our_netif(ETH_NETIF_DESC, event->esp_netif)) {
        return;
    }
    ESP_LOGI(TAG, "Got IPv4 event: Interface \"%s\" address: " IPSTR, esp_netif_get_desc(event->esp_netif), IP2STR(&event->ip_info.ip));
    //xSemaphoreGive(s_semph_get_ip_addrs);
}

#if CONFIG_EXAMPLE_CONNECT_IPV6

static void eth_on_got_ipv6(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data)
{
    ip_event_got_ip6_t *event = (ip_event_got_ip6_t *)event_data;
    if (!is_our_netif(ETH_NETIF_DESC, event->esp_netif)) {
        return;
    }
    esp_ip6_addr_type_t ipv6_type = esp_netif_ip6_get_addr_type(&event->ip6_info.ip);
    ESP_LOGI(TAG, "Got IPv6 event: Interface \"%s\" address: " IPV6STR ", type: %s", esp_netif_get_desc(event->esp_netif),
             IPV62STR(event->ip6_info.ip), example_ipv6_addr_types_to_str[ipv6_type]);
    if (ipv6_type == EXAMPLE_CONNECT_PREFERRED_IPV6_TYPE) {
        //xSemaphoreGive(s_semph_get_ip6_addrs);
    }
}


/*
static void on_eth_event(void *esp_netif, esp_event_base_t event_base,
                         int32_t event_id, void *event_data)
{
    switch (event_id) {
    case ETHERNET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Up");
        ESP_ERROR_CHECK(esp_netif_create_ip6_linklocal(esp_netif));
        break;
    default:
        break;
    }
}

*/

#endif // CONFIG_EXAMPLE_CONNECT_IPV6


static void eth_event_handler(void *esp_netif, esp_event_base_t event_base,
                              int32_t event_id, void *event_data)
{

    switch (event_id) {
    case ETHERNET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Up");
        ESP_ERROR_CHECK(esp_netif_create_ip6_linklocal(esp_netif));
        break;
    case ETHERNET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "Ethernet Link Down");
        //s_ethernet_is_connected = false;
        //ESP_ERROR_CHECK(esp_wifi_stop());// Cambiar por reconfiguración Wifi
        break;
    case ETHERNET_EVENT_START:
        ESP_LOGE(TAG, "Ethernet Started Función de Identificación");
        break;
    case ETHERNET_EVENT_STOP:
        ESP_LOGI(TAG, "Ethernet Stopped");
        break;
    default:
        ESP_LOGW(TAG, "Evento Ethernet diferente, posible GOT_IP");
        break;
    }
}



void eth_start(flash_eth_t *flash_eth)
{
      //***Solo L2
    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    mac_config.rx_task_stack_size = CONFIG_ETH_EMAC_TASK_STACK_SIZE;                       //Solo L3
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = CONFIG_ETH_PHY_ADDR;
    phy_config.reset_gpio_num = CONFIG_ETH_PHY_RST_GPIO;

    gpio_install_isr_service(0);
    spi_bus_config_t buscfg = {
        .miso_io_num = CONFIG_ETH_SPI_MISO_GPIO,
        .mosi_io_num = CONFIG_ETH_SPI_MOSI_GPIO,
        .sclk_io_num = CONFIG_ETH_SPI_SCLK_GPIO,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(CONFIG_ETH_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));//L2 en l3 SPI_DMA_CH_AUTO es 1

    spi_device_interface_config_t spi_devcfg = {
        .mode = 0,
        .clock_speed_hz = CONFIG_ETH_SPI_CLOCK_MHZ * 1000 * 1000,
        .spics_io_num = CONFIG_ETH_SPI_CS_GPIO,
        .queue_size = 20
    };

    eth_w5500_config_t w5500_config = ETH_W5500_DEFAULT_CONFIG(CONFIG_ETH_SPI_HOST, &spi_devcfg);
    w5500_config.int_gpio_num = CONFIG_ETH_SPI_INT_GPIO;
    s_mac = esp_eth_mac_new_w5500(&w5500_config, &mac_config); 
    s_phy = esp_eth_phy_new_w5500(&phy_config);
    esp_eth_config_t config = ETH_DEFAULT_CONFIG(s_mac, s_phy);


    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &s_eth_handle));

    char mac_addr_str[] = "1:02:FF:03:04:05";
    uint8_t mac_addr[6];
    sscanf(mac_addr_str, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &mac_addr[0], &mac_addr[1], &mac_addr[2], &mac_addr[3], &mac_addr[4], &mac_addr[5]);
    ESP_ERROR_CHECK(esp_eth_ioctl(s_eth_handle, ETH_CMD_S_MAC_ADDR, mac_addr ));
    
/*Inicio Respaldo L3*/

    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_ETH();
    esp_netif_config.if_desc = ETH_NETIF_DESC;
    esp_netif_config.route_prio = 64;
    esp_netif_config_t netif_config = {
        .base = &esp_netif_config,
        .stack = ESP_NETIF_NETSTACK_DEFAULT_ETH
    };
    esp_netif_t *netif = esp_netif_new(&netif_config);
    assert(netif);

    s_eth_glue = esp_eth_new_netif_glue(s_eth_handle);
    esp_netif_attach(netif, s_eth_glue);
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &eth_on_got_ip, NULL));




    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, eth_event_handler, netif));
    //ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ETHERNET_EVENT_CONNECTED, &on_eth_event, netif));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_GOT_IP6, &eth_on_got_ipv6, NULL));
    esp_err_t ret=esp_eth_start(s_eth_handle);
    
    if (ret==ESP_OK)
    {
        ESP_LOGI(TAG, "Configuración Ethernet finalizada, esperando direcciones IP");
        ESP_ERROR_CHECK(esp_register_shutdown_handler(&ethernet_shutdown));
    }else{
        ESP_LOGE(TAG, "Error en configuración Ethernet %s", esp_err_to_name(ret));
    }
    //free(flash_eth);
    vTaskDelete(NULL);
}





static void eth_stop(void)
{
    esp_netif_t *eth_netif = get_netif_from_desc(ETH_NETIF_DESC);


    ESP_ERROR_CHECK(esp_event_handler_unregister(ETH_EVENT, ESP_EVENT_ANY_ID, eth_event_handler));
    
    
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_ETH_GOT_IP, &eth_on_got_ip));
#if CONFIG_EXAMPLE_CONNECT_IPV6
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_GOT_IP6, &eth_on_got_ipv6));
    //ESP_ERROR_CHECK(esp_event_handler_unregister(ETH_EVENT, ETHERNET_EVENT_CONNECTED, &on_eth_event));
#endif
    ESP_ERROR_CHECK(esp_eth_stop(s_eth_handle));
    ESP_ERROR_CHECK(esp_eth_del_netif_glue(s_eth_glue));
    ESP_ERROR_CHECK(esp_eth_driver_uninstall(s_eth_handle));
    s_eth_handle = NULL;
    ESP_ERROR_CHECK(s_phy->del(s_phy));
    ESP_ERROR_CHECK(s_mac->del(s_mac));

    esp_netif_destroy(eth_netif);
}

esp_eth_handle_t get_eth_handle(void)
{
    return s_eth_handle;
}

/* tear down connection, release resources */
void ethernet_shutdown(void)
{
    if (s_semph_get_ip_addrs == NULL) {
        return;
    }
    vSemaphoreDelete(s_semph_get_ip_addrs);
    s_semph_get_ip_addrs = NULL;
#if CONFIG_EXAMPLE_CONNECT_IPV6
    vSemaphoreDelete(s_semph_get_ip6_addrs);
    s_semph_get_ip6_addrs = NULL;
#endif
    eth_stop();
}

/*FUNCIIÓN PARA VALIDAR CONEXIÓN DE ETHERNET, USANDO SEMÁFOROS
void ethernet_init(void)
{
    s_semph_get_ip_addrs = xSemaphoreCreateBinary();
    if (s_semph_get_ip_addrs == NULL) {

        ESP_LOGI(TAG, "ESP_ERR_NO_MEM");
        vTaskDelete(NULL);
        //return ESP_ERR_NO_MEM;
        //Eliminar semaforo como en ipv6
    }
#if CONFIG_EXAMPLE_CONNECT_IPV6
    s_semph_get_ip6_addrs = xSemaphoreCreateBinary();
    if (s_semph_get_ip6_addrs == NULL) {
        vSemaphoreDelete(s_semph_get_ip_addrs);
        ESP_LOGI(TAG, "ESP_ERR_NO_MEM");
        vTaskDelete(NULL);
        //return ESP_ERR_NO_MEM;
    }
#endif

    eth_start();
    ESP_LOGW(TAG, "Esperando direcciones IP(s). de Etherner");
    xSemaphoreTake(s_semph_get_ip_addrs, portMAX_DELAY);
#if CONFIG_EXAMPLE_CONNECT_IPV6
    ESP_LOGW(TAG, "Esperando direcciones IP(s). de Etherner IPV6");
    xSemaphoreTake(s_semph_get_ip6_addrs, portMAX_DELAY);
#endif
    //reducir valores de los semáforos y validar valores de retorno
    //aquí se puede configurar direcciones estáticas
    print_all_netif_ips(ETH_NETIF_DESC);
    vTaskDelete(NULL);
    //return ESP_OK;
}
*/