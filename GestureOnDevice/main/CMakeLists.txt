
idf_component_register(
    SRCS
        "inference_responder.cc"
        "image_provider.cc"
        "main.cc"
        "main_functions.cc"
        "model_settings.cc"
        "gesture_model_data.cc"
        "app_camera_esp.c"
        "esp_lcd.c"
        "app_button.c"

    PRIV_REQUIRES console tflite-lib esp32-camera screen spi_flash fb_gfx esp_adc_cal
    INCLUDE_DIRS "")
