set(embed_files "web/www/index_ov2640.html.gz"
                "web/www/index_ov3660.html.gz"
                "web/www/index_ov5640.html.gz"
                "web/www/monitor.html.gz")

set(src_dirs    
                camera
                web)

set(include_dirs    
                    camera
                    web)

set(requires    esp32-camera
                bus
                esp_http_server
                nvs_flash
                mdns
                esp_adc_cal)

idf_component_register(SRC_DIRS ${src_dirs} INCLUDE_DIRS ${include_dirs} REQUIRES ${requires} EMBED_FILES ${embed_files})

component_compile_options(-ffast-math -O3)
