#include "esp_event.h"
#include "network.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_log.h"

#include "files.h"
#include "wserver.h"


//#include "flash.h"

static const char *TAG="Safervi";

void app_main(void)
{



get_file("/spiffs/network/index.html");


    ESP_LOGI(TAG, "Iniciando programa, Leyendo Archivo");



    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    ESP_ERROR_CHECK(esp_netif_init());//revisar si es necessario
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(network_connect());

#if CONFIG_EXAMPLE_CONNECT_WIFI
    esp_wifi_set_ps(WIFI_PS_NONE);
#endif // CONFIG_EXAMPLE_CONNECT_WIFI
      
    printf("SafervI! Sistemas de Seguridad y Domótica\n");

/*
    FUNCIONES AST
    flash_ast_t ast_config = {};
    ESP_LOGE(TAG, "Estado de GET %s", esp_err_to_name(flash_ast_get(&ast_config)));
    flash_ast_t ast_rec = {};
    ESP_LOGE(TAG, "Estado de GET %s",esp_err_to_name(flash_ast_write(flash_ast_get(&ast_config))));
    */


   start_wserver();


}