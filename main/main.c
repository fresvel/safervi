//#include <string.h>
//#include <inttypes.h>
//#include "freertos/FreeRTOS.h"
//#include "freertos/task.h"
//#include "esp_system.h"
#include "esp_event.h"
//#include "esp_log.h"
//#include "esp_ota_ops.h"
//#include "esp_app_format.h"
//#include "esp_http_client.h"
//#include "esp_flash_partitions.h"
//#include "esp_partition.h"

//#include "driver/gpio.h"

//#include "errno.h"

#include "network.h"
#include "nvs.h"
#include "nvs_flash.h"

void app_main(void)
{
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
    /* Ensure to disable any WiFi power save mode, this allows best throughput
     * and hence timings for overall OTA operation.
     */
    esp_wifi_set_ps(WIFI_PS_NONE);
#endif // CONFIG_EXAMPLE_CONNECT_WIFI
    printf("SafervI! Sistemas de Seguridad y Domótica\n");

}