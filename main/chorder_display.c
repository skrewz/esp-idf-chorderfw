#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include "esp_log.h"
#include "chorder_display.h"
#include "config.h"

void initialize_lcd() {
  ESP_LOGI(TAG, "Initializing SPIFFS");

  esp_vfs_spiffs_conf_t conf = {
    .base_path = "/spiffs",
    .partition_label = NULL,
    .max_files = 8,
    .format_if_mount_failed =true
  };

  // Use settings defined above toinitialize and mount SPIFFS filesystem.
  // Note: esp_vfs_spiffs_register is anall-in-one convenience function.
  esp_err_t ret =esp_vfs_spiffs_register(&conf);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      ESP_LOGE(TAG, "Failed to mount or format filesystem");
    } else if (ret == ESP_ERR_NOT_FOUND) {
      ESP_LOGE(TAG, "Failed to find SPIFFS partition");
    } else {
      ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)",esp_err_to_name(ret));
    }
    return;
  }

  size_t total = 0, used = 0;
  ret = esp_spiffs_info(NULL, &total,&used);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG,"Failed to get SPIFFS partition information (%s)",esp_err_to_name(ret));
  } else {
    ESP_LOGI(TAG,"Partition size: total: %d, used: %d", total, used);
  }

  SPIFFS_Directory("/spiffs/");
}
void SPIFFS_Directory(char * path) {
  DIR* dir = opendir(path);
  assert(dir != NULL);
  while (true) {
    struct dirent*pe = readdir(dir);
    if (!pe) break;
    ESP_LOGI(TAG,"d_name=%s d_ino=%d d_type=%x", pe->d_name,pe->d_ino, pe->d_type);
  }
  closedir(dir);
}

void clear_lcd(TFT_t * dev, uint16_t color) {
  lcdFillScreen(dev, color);
}

void render_message(TFT_t * dev, FontxFile *fx, uint16_t color, int x_off, int y_off, int direction, unsigned char *message) {
  // get font width & height
  uint8_t buffer[FontxGlyphBufSize];
  uint8_t fontWidth;
  uint8_t fontHeight;
  GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
  ESP_LOGI(__FUNCTION__,"fontWidth=%d, total_width=%d, fontHeight=%d, total_height=%d",fontWidth,CONFIG_WIDTH,fontHeight,CONFIG_HEIGHT);

  unsigned char buf[CONFIG_WIDTH/fontWidth+1]; // space for '\0'
  size_t offset_into_string = 0;
  size_t line_number = 0;
  size_t index_into_line;

  lcdSetFontDirection(dev, direction);
  while(offset_into_string <= strlen((char *)message))
  {
    buf[0] = '\0';
    for(index_into_line = 0; index_into_line < CONFIG_WIDTH/fontWidth; index_into_line++)
    {
      buf[index_into_line+1] = '\0';
      if ('\n' == message[offset_into_string+index_into_line] || '\0' == message[offset_into_string+index_into_line])
      {
        index_into_line++;
        break;
      }
      buf[index_into_line] = message[offset_into_string+index_into_line];
    }
    offset_into_string += index_into_line;
    lcdDrawString(dev, fx, 0, (line_number * fontHeight) + fontHeight-1, buf, color);
    line_number++;
  }
}
