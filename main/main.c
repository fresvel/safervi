#include "esp_event.h"
#include "network.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include "flash.h"

static const char *TAG="Safervi";

void app_main(void)
{

    ESP_LOGI(TAG, "Iniciando programa, Leyendo Archivo");
    //ESP_LOGI(TAG, "%s", index_html);

/* LLAMADA A LAS FUNCIONES DE RED CAMBIAR A TAREAS

    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());


    ESP_ERROR_CHECK(network_connect());

#if CONFIG_EXAMPLE_CONNECT_WIFI
    esp_wifi_set_ps(WIFI_PS_NONE);
#endif // CONFIG_EXAMPLE_CONNECT_WIFI
    
  */  
    printf("SafervI! Sistemas de Seguridad y Domótica\n");


    //show_file(); SPIFS

    ESP_LOGI(TAG, "Estado de init%s", esp_err_to_name(flash_network_init()));
    
    flash_sta_t sta_config = {};
    ESP_LOGE(TAG, "Estado de GET %s", esp_err_to_name(flash_sta_get(&sta_config)));






    flash_sta_t* flash_sta;
    size_t req_size = sizeof(flash_sta_t);
    flash_sta = malloc(req_size);

    strcpy((char*)flash_sta->ssid, "Fresvel");
    //sprintf(flash_sta->ssid, "Kira");
    strcpy((char*)flash_sta->password, "FresvelSafervi");
    //sprintf(flash_sta->password, "KiraFresvel");
    flash_sta->max_retry = 12;
    
    
    
    
    ESP_LOGE(TAG, "Estado de write %s",esp_err_to_name(flash_sta_write(flash_sta)));

    




    /*FUNCIONES AST
    flash_ast_t ast_config = {};
    ESP_LOGE(TAG, "Estado de GET %s", esp_err_to_name(flash_ast_get(&ast_config)));
    flash_ast_t ast_rec = {};
    ESP_LOGE(TAG, "Estado de GET %s",esp_err_to_name(flash_ast_write(flash_ast_get(&ast_config))));
    */

}