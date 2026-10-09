idf_component_register(
    SRCS "main.c"
         "tianshang_sim.c"
         "tianshang_store.c"
         "tianshang_ui.c"
         "tianshang_audio.c"
         "tianshang_sfx_data.c"
         "tianshang_img_assets.c"
         "font_tianshang_14.c"
         "font_tianshang_20.c"
    INCLUDE_DIRS "."
    REQUIRES bsp esp_event esp_hw_support esp_timer nvs_flash
)
