set(srcs "flash_network.c"
         "flash_protocol.c")


idf_component_register(SRCS "${srcs}" 
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_netif esp_wifi nvs_flash spiffs)


spiffs_create_partition_image(storage ../../files FLASH_IN_PROJECT)