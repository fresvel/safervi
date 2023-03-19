set(srcs "network.c"
         "wifi.c"
         "ethernet.c")

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES esp_netif driver esp_wifi vfs esp_eth)

