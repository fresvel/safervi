idf_component_register(SRCS "flash_network.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_netif esp_wifi nvs_flash)
