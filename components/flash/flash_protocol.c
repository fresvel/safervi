#include <files.h>
#include <esp_spiffs.h>
#include <esp_log.h>
#include <string.h>
#include <esp_system.h>
//#include "esp_heap_caps.h"

static const char*TAG = "Files";

void get_file(char* file_path){

 esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 10,
      .format_if_mount_failed = true
    };


    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return;
    }else{
        ESP_LOGI(TAG, "Sistema de Archivos montado de forma correcta");
    }

    ESP_LOGI(TAG, "Performing SPIFFS_check().");
    ret = esp_spiffs_check(conf.partition_label);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPIFFS_check() failed (%s)", esp_err_to_name(ret));
        return;
    } else {
        ESP_LOGI(TAG, "SPIFFS_check() successful");
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s). Formatting...", esp_err_to_name(ret));
        esp_spiffs_format(conf.partition_label);
        return;
    } else {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }    



    size_t heap_size = esp_get_free_heap_size();
    printf("Heap size: %d bytes\n", heap_size);

        




    ESP_LOGI(TAG, "Reading hello.txt");
    FILE* fhello = fopen(file_path, "r");
    if (fhello == NULL) {
        ESP_LOGE(TAG, "Failed to open hello.txt");
        return;
    }


    fseek(fhello, 0, SEEK_END);
    long fsize = ftell(fhello)+1;
    rewind(fhello);
    ESP_LOGI(TAG, "Tamaño del hello.txt%ld bytes\n", fsize);
    size_t block_size = (size_t)heap_size/4;//25% de la memoria libre
    if(fsize>block_size){
        for(int i=0; i<=(int)(fsize/block_size); i++){
            char buf[block_size+1];
            memset(buf, 0, block_size+1);
            fread(buf, 1, block_size, fhello);
            printf("%s\n", buf);
        }
    }else{
    char buf[fsize];
    memset(buf, 0, fsize);
    fread(buf, 1, fsize, fhello);
    printf("%s", buf);
    }




        fclose(fhello);
    
    // Display the read contents from the file
    





    


    esp_vfs_spiffs_unregister(conf.partition_label);
    ESP_LOGI(TAG, "Sistema de archivos desmontado");


}

void set_file(void*){

}