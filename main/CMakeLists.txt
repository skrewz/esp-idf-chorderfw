set(COMPONENT_SRCS
  main.c
  st7789.c
  chorder_display.c
  esp_hidd_prf_api.c
  hid_dev.c
  hid_device_le_prf.c
  decode_image.c
  fontx.c
  pngle.c)
set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()
spiffs_create_partition_image(storage ../font FLASH_IN_PROJECT)
