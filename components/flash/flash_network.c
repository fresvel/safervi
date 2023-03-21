#include <stdio.h>
#include "flash.h"
#include <esp_log.h>


static const char *TAG="NVS";

esp_err_t flash_sta_get(flash_sta_t* flash_sta){
    nvs_handle_t flash_handle;
    esp_err_t err;

    err = nvs_open_from_partition("network", "sta",NVS_READONLY,&flash_handle);
    if (err != ESP_OK){
        if (err == ESP_ERR_NVS_NOT_FOUND){
            ESP_LOGW(TAG, "Memoria vacía");
            return ESP_OK;
        }else return err;
    }
        
    size_t req_size = sizeof(flash_sta_t);

    flash_sta = malloc(req_size);

    err = nvs_get_blob(flash_handle, "sta", flash_sta, &req_size);
    if (err != ESP_OK) {
        free(flash_sta);
        return err;
    }
    
    printf("ssid:%s  passwd: %s max: %ld\n", flash_sta->ssid, flash_sta->password, flash_sta->max_retry);
    
    free(flash_sta);
    
    nvs_close(flash_handle);
    return ESP_OK;
}


esp_err_t flash_sta_write(flash_sta_t*flash_sta){
    nvs_handle_t flash_handle;
    esp_err_t err;

    // Open
    err = nvs_open_from_partition("network","sta", NVS_READWRITE, &flash_handle);
    if (err != ESP_OK) return err;

    size_t req_size = sizeof(flash_sta_t);
    err = nvs_set_blob(flash_handle, "sta", flash_sta, req_size);
    free(flash_sta);

    if (err != ESP_OK) return err;

    // Commit
    err = nvs_commit(flash_handle);
    if (err != ESP_OK) return err;

    // Close
    nvs_close(flash_handle);
    return ESP_OK;
}

esp_err_t flash_ap_get(flash_sta_t*flash_ap){
    return ESP_OK;
}

esp_err_t flash_ap_write(flash_sta_t*flash_ap){
    return ESP_OK;
}

esp_err_t flash_eth_get(flash_eth_t*flash_eth){
    return ESP_OK;
}

esp_err_t flash_eth_write(flash_eth_t*flash_eth){
    return ESP_OK;
}




//Validar la creacion de de vectores flash_ast_t * id[n]

esp_err_t flash_ast_get(flash_ast_t* flash_ast){
    nvs_handle_t flash_handle;
    esp_err_t err;

    err = nvs_open_from_partition("network", "ast",NVS_READONLY,&flash_handle);
    if (err != ESP_OK) return err;

    size_t req_size = 0;
    err = nvs_get_blob(flash_handle, "ast", NULL, &req_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;

    ESP_LOGI(TAG, "Datos obtenidos");
    if (req_size == 0) {
        printf("Memoria vacía\n");
    } else {
        flash_ast = malloc(req_size);
        err = nvs_get_blob(flash_handle, "ast", flash_ast, &req_size);
        if (err != ESP_OK) {
            free(flash_ast);
            return err;
        }
        
        printf("ssid:%s  passwd: %s max: %ld\n", flash_ast->ssid, flash_ast->password, flash_ast->max_retry);
        
        free(flash_ast);
            }
    nvs_close(flash_handle);
    return ESP_OK;
}


esp_err_t flash_ast_write(flash_ast_t*flash_ast){
    nvs_handle_t flash_handle;
    esp_err_t err;

    // Open
    err = nvs_open_from_partition("network","ast", NVS_READWRITE, &flash_handle);
    if (err != ESP_OK) return err;

    printf("partición abierta\n");


    size_t req_size = 0;
    err = nvs_get_blob(flash_handle, "ast", NULL, &req_size);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) return err;

    
    flash_ast = malloc(req_size + sizeof(flash_ast_t));
    if (req_size > 0) {
        err = nvs_get_blob(flash_handle, "ast", flash_ast, &req_size);
        if (err != ESP_OK) {
            free(flash_ast);
            return err;
        }
    }

    // Write value including previously saved blob if available
    req_size += sizeof(flash_ast_t);
    //required_size += sizeof(cat_t);

    strcpy((char*)flash_ast->ssid, "Kira");
    //sprintf(flash_sta->ssid, "Kira");
    strcpy((char*)flash_ast->password, "password");
    //sprintf(flash_sta->password, "KiraFresvel");
    flash_ast->max_retry = 12;
    
    err = nvs_set_blob(flash_handle, "ast", flash_ast, req_size);
    free(flash_ast);

    if (err != ESP_OK) return err;

    // Commit
    err = nvs_commit(flash_handle);
    if (err != ESP_OK) return err;

    // Close
    nvs_close(flash_handle);
    return ESP_OK;
}









esp_err_t flash_network_init(){
    
    ESP_LOGI(TAG, "Iniciando partición de Network");

    esp_err_t err = nvs_flash_init_partition("network");

    if(err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND){
        ESP_ERROR_CHECK(nvs_flash_erase_partition("network"));
        err= nvs_flash_init_partition("network");
    }
    return err;
}

void show_file(void)
{
ESP_LOGI(TAG, "Memoria no Volatil");

    extern const uint8_t index_html[] asm("_binary_index_html_start");
    ESP_LOGW(TAG, "Archivo HTML: %s", index_html);

}
