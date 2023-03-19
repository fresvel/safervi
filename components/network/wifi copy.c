#include <string.h>
#include "network.h"
#include "private_network.h"
#include "esp_log.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sys.h"


static const char *TAG = "WIFI";


#if CONFIG_STA_ENABLE


static esp_netif_t *s_sta_netif = NULL;
static SemaphoreHandle_t s_semph_get_ip_addrs = NULL;
#if CONFIG_STA_CONNECT_IPV6
static SemaphoreHandle_t s_semph_get_ip6_addrs = NULL;
#endif

#if CONFIG_STA_WIFI_SCAN_METHOD_FAST
#define STA_WIFI_SCAN_METHOD WIFI_FAST_SCAN
#elif CONFIG_STA_WIFI_SCAN_METHOD_ALL_CHANNEL
#define STA_WIFI_SCAN_METHOD WIFI_ALL_CHANNEL_SCAN
#endif

#if CONFIG_STA_WIFI_CONNECT_AP_BY_SIGNAL
#define STA_WIFI_CONNECT_AP_SORT_METHOD WIFI_CONNECT_AP_BY_SIGNAL
#elif CONFIG_STA_WIFI_CONNECT_AP_BY_SECURITY
#define STA_WIFI_CONNECT_AP_SORT_METHOD WIFI_CONNECT_AP_BY_SECURITY
#endif

#if CONFIG_STA_WIFI_AUTH_OPEN
#define STA_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_STA_WIFI_AUTH_WEP
#define STA_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_STA_WIFI_AUTH_WPA_PSK
#define STA_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_STA_WIFI_AUTH_WPA2_PSK
#define STA_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_STA_WIFI_AUTH_WPA_WPA2_PSK
#define STA_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_STA_WIFI_AUTH_WPA2_ENTERPRISE
#define STA_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_ENTERPRISE
#elif CONFIG_STA_WIFI_AUTH_WPA3_PSK
#define STA_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_STA_WIFI_AUTH_WPA2_WPA3_PSK
#define STA_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_STA_WIFI_AUTH_WAPI_PSK
#define STA_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

static int s_retry_num = 0;

static void sta_handler_disconnect(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data){
    s_retry_num++;
    if (s_retry_num > CONFIG_STA_WIFI_CONN_MAX_RETRY) {
        ESP_LOGE(TAG, "Falló la conexión Wifi %d intentos", s_retry_num);
        if (s_semph_get_ip_addrs) {
            xSemaphoreGive(s_semph_get_ip_addrs);
        }
#if CONFIG_STA_CONNECT_IPV6
        if (s_semph_get_ip6_addrs) {
            xSemaphoreGive(s_semph_get_ip6_addrs);
        }
#endif
        return;
    }
    ESP_LOGW(TAG, "Wi-Fi desconectado, intentando reconctar...");
    esp_err_t err = esp_wifi_connect();
    if (err == ESP_ERR_WIFI_NOT_STARTED) {
        return;
    }
    ESP_ERROR_CHECK(err);
}

static void sta_handler_connect( void *esp_netif, esp_event_base_t event_base,
                                            int32_t event_id, void *event_data){
#if CONFIG_STA_CONNECT_IPV6
    esp_netif_create_ip6_linklocal(esp_netif);
#endif // CONFIG_STA_CONNECT_IPV6
}

static void sta_handler_got_ip(void *arg, esp_event_base_t event_base,
                      int32_t event_id, void *event_data)
{
    s_retry_num = 0;
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    if (!is_our_netif(STA_NETIF_DESC, event->esp_netif)) {
        return;
    }
    ESP_LOGI(TAG, "Evento IPv4: Interfaz \"%s\" dirección: " IPSTR, esp_netif_get_desc(event->esp_netif), IP2STR(&event->ip_info.ip));
    
    if (s_semph_get_ip_addrs) {
        xSemaphoreGive(s_semph_get_ip_addrs);
    } else {
        ESP_LOGW(TAG, "- *********************IPv4 address: " IPSTR ",", IP2STR(&event->ip_info.ip));
    }

    //Validar posibilidad de intergrar Wifi AP para identificar la IP de las estaciones
}

#if CONFIG_STA_CONNECT_IPV6
static void sta_handler_got_ipv6(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data){
    ip_event_got_ip6_t *event = (ip_event_got_ip6_t *)event_data;
    if (!is_our_netif(STA_NETIF_DESC, event->esp_netif)) {
        return;
    }
    esp_ip6_addr_type_t ipv6_type = esp_netif_ip6_get_addr_type(&event->ip6_info.ip);
    ESP_LOGI(TAG, "Got IPv6 event: Interface \"%s\" address: " IPV6STR ", type: %s", esp_netif_get_desc(event->esp_netif),
             IPV62STR(event->ip6_info.ip), example_ipv6_addr_types_to_str[ipv6_type]);

    if (ipv6_type == STA_CONNECT_PREFERRED_IPV6_TYPE) {
        if (s_semph_get_ip6_addrs) {
            xSemaphoreGive(s_semph_get_ip6_addrs);
        } else {
            ESP_LOGI(TAG, "- IPv6 address: " IPV6STR ", type: %s", IPV62STR(event->ip6_info.ip), example_ipv6_addr_types_to_str[ipv6_type]);
        }
    }
    //Validar posibilidad de intergrar Wifi AP para identificar la IP de las estaciones
}
#endif // CONFIG_STA_CONNECT_IPV6





esp_err_t wifi_connect_sta(wifi_config_t wifi_config, bool wait)
{
    if (wait) {
        s_semph_get_ip_addrs = xSemaphoreCreateBinary();
        if (s_semph_get_ip_addrs == NULL) {
            return ESP_ERR_NO_MEM;
        }
#if CONFIG_STA_CONNECT_IPV6
        s_semph_get_ip6_addrs = xSemaphoreCreateBinary();
        if (s_semph_get_ip6_addrs == NULL) {
            vSemaphoreDelete(s_semph_get_ip_addrs);
            return ESP_ERR_NO_MEM;
        }
#endif
    }
    s_retry_num = 0;
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &sta_handler_disconnect, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &sta_handler_got_ip, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &sta_handler_connect, s_sta_netif));
#if CONFIG_STA_CONNECT_IPV6
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_GOT_IP6, &sta_handler_got_ipv6, NULL));
#endif

    ESP_LOGI(TAG, "Conectando a %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    esp_err_t ret = esp_wifi_connect();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Error en conexión WiFi! ret:%x", ret);
        return ret;
    }
    
    if (wait) {
        ESP_LOGI(TAG, "Waiting for IP(s)");
        xSemaphoreTake(s_semph_get_ip_addrs, portMAX_DELAY);
        ESP_LOGE(TAG, "Dirección Obtenida");
#if CONFIG_STA_CONNECT_IPV6
        xSemaphoreTake(s_semph_get_ip6_addrs, portMAX_DELAY);
#endif
        if (s_retry_num > CONFIG_STA_WIFI_CONN_MAX_RETRY) {
            ESP_LOGW(TAG, "La tarjeta no pudo establecer conexión con el Punto de Acceso");
            return ESP_OK;
        }
    }
    return ESP_OK;
}

esp_err_t wifi_disconnect_sta(void)
{
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &sta_handler_disconnect));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &sta_handler_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &sta_handler_connect));
#if CONFIG_STA_CONNECT_IPV6
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_GOT_IP6, &sta_handler_got_ipv6));
#endif
    if (s_semph_get_ip_addrs) {
        vSemaphoreDelete(s_semph_get_ip_addrs);
    }
#if CONFIG_STA_CONNECT_IPV6
    if (s_semph_get_ip6_addrs) {
        vSemaphoreDelete(s_semph_get_ip6_addrs);
    }
#endif
    return esp_wifi_disconnect();
}


void wifi_stop_driver(void){
    esp_err_t err = esp_wifi_stop();
    if (err == ESP_ERR_WIFI_NOT_INIT) {
        return;
    }
    ESP_ERROR_CHECK(err);
    ESP_ERROR_CHECK(esp_wifi_deinit());

}

void wifi_stop_sta(void)
{
    ESP_ERROR_CHECK(esp_wifi_clear_default_wifi_driver_and_handlers(s_sta_netif));
    esp_netif_destroy(s_sta_netif);
    s_sta_netif = NULL;
}

#endif /* CONFIG_STA_ENABLE*/

/////////////////////////////////////////////////////////////////////
#if CONFIG_AP_ENABLE

#if CONFIG_AP_WIFI_AUTH_OPEN
#define AP_WIFI_AUTH_MODE WIFI_AUTH_OPEN
/*
#elif CONFIG_AP_WIFI_AUTH_WEP
#define AP_WIFI_AUTH_MODE WIFI_AUTH_WEP
#elif CONFIG_AP_WIFI_AUTH_WPA_PSK
#define AP_WIFI_AUTH_MODE WIFI_AUTH_WPA_PSK
#elif CONFIG_AP_WIFI_AUTH_WPA2_PSK
#define AP_WIFI_AUTH_MODE WIFI_AUTH_WPA2_PSK
#elif CONFIG_AP_WIFI_AUTH_WPA_WPA2_PSK
#define AP_WIFI_AUTH_MODE WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_AP_WIFI_AUTH_WPA2_ENTERPRISE
#define AP_WIFI_AUTH_MODE WIFI_AUTH_WPA2_ENTERPRISE
#elif CONFIG_AP_WIFI_AUTH_WPA3_PSK
#define AP_WIFI_AUTH_MODE WIFI_AUTH_WPA3_PSK
#elif CONFIG_AP_WIFI_AUTH_WPA2_WPA3_PSK
#define AP_WIFI_AUTH_MODE WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_AP_WIFI_AUTH_WAPI_PSK
#define AP_WIFI_AUTH_MODE WIFI_AUTH_WAPI_PSK
*/
#endif

static esp_netif_t *s_ap_netif = NULL;


static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}


void wifi_stop_ap(void){
    
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler));
    esp_netif_destroy(s_ap_netif);
    s_ap_netif = NULL;
}


#endif


esp_err_t wifi_driver_init(wifi_mode_t mode){

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(mode));
    wifi_config_t wifi_config = {};
    ESP_LOGI(TAG, "Iniciando driver de Wifi");

    /*AP*/
    #if CONFIG_AP_ENABLE
    if(mode == WIFI_MODE_AP||mode == WIFI_MODE_APSTA){
        ESP_LOGI(TAG, "Configuración de Punto de Acceso");
        
        esp_netif_inherent_config_t ap_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_AP();
        ap_netif_config.if_desc = AP_NETIF_DESC;
        ap_netif_config.route_prio = 128;
        s_ap_netif = esp_netif_create_wifi(WIFI_IF_AP, &ap_netif_config);

        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler,NULL,NULL));/*aquí*/
        







        strcpy((char*)wifi_config.ap.ssid, CONFIG_AP_WIFI_SSID);
        strcpy((char*)wifi_config.ap.password, CONFIG_AP_WIFI_PASSWORD);
        wifi_config.ap.ssid_len= strlen(CONFIG_AP_WIFI_SSID);
        wifi_config.ap.channel = CONFIG_AP_WIFI_CHANNEL;
        wifi_config.ap.max_connection = CONFIG_AP_MAX_STA_CONN;
        wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;//AP_WIFI_AUTH_MODE;
        wifi_config.ap.pmf_cfg.required = false;
        if (strlen(CONFIG_AP_WIFI_PASSWORD) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
        }
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    }
    #endif
    /*AP*/
    
    /*STA*/
    #if CONFIG_STA_ENABLE
    if(mode == WIFI_MODE_STA||mode == WIFI_MODE_APSTA){
        esp_netif_inherent_config_t sta_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
        sta_netif_config.if_desc = STA_NETIF_DESC;
        sta_netif_config.route_prio = 128;
        s_sta_netif = esp_netif_create_wifi(WIFI_IF_STA, &sta_netif_config);
        esp_wifi_set_default_wifi_sta_handlers();
        ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    }
    #endif
    /*STA*/

    ESP_ERROR_CHECK(esp_wifi_start());
    
    /*AP*/
    #if CONFIG_AP_ENABLE
    if(mode == WIFI_MODE_AP||mode == WIFI_MODE_APSTA)
    ESP_LOGI(TAG, "Punto de acceso configurado. SSID:%s password:%s channel:%d ",
             CONFIG_AP_WIFI_SSID, CONFIG_AP_WIFI_PASSWORD, CONFIG_AP_WIFI_CHANNEL);
    #endif
    /*AP*/

    /*STA*/
    #if CONFIG_STA_ENABLE
    if(mode == WIFI_MODE_STA||mode == WIFI_MODE_APSTA){
        strcpy((char*)wifi_config.sta.ssid, CONFIG_STA_WIFI_SSID);
        strcpy((char*)wifi_config.sta.password, CONFIG_STA_WIFI_PASSWORD);
        wifi_config.sta.scan_method = STA_WIFI_SCAN_METHOD;
        wifi_config.sta.sort_method = STA_WIFI_CONNECT_AP_SORT_METHOD;
        wifi_config.sta.threshold.rssi = CONFIG_STA_WIFI_SCAN_RSSI_THRESHOLD;
        wifi_config.sta.threshold.authmode = STA_WIFI_SCAN_AUTH_MODE_THRESHOLD;
        ESP_LOGW(TAG, "Estableciendo conexión de estación");
        return wifi_connect_sta(wifi_config, true);
    }
    #endif
    /*STA*/
    return ESP_OK; //Retorno de AP   
}


void wifi_shutdown_driver(wifi_mode_t mode_off)
{

    switch (mode_off) {
        case WIFI_MODE_STA:
            wifi_disconnect_sta();
            wifi_stop_driver();
            wifi_stop_sta();

        break;
        case WIFI_MODE_AP:
            wifi_stop_driver();
            wifi_stop_ap();
        break;
        case WIFI_MODE_APSTA:
            //Posiblemente se necesite wifi set mode null or nan
            wifi_disconnect_sta();
            wifi_stop_sta();
            wifi_stop_ap();
        break;
        default: 
        break;
    }

    //Crear función para encender wifi nuevamente llamando a wifi_driver_init(modo)
    //Leer modo desde la memoria 
}