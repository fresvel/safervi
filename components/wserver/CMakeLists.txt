idf_component_register(SRCS "keep_alive.c" "wserver.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_timer esp_https_server esp_netif esp_eth esp_wifi)
