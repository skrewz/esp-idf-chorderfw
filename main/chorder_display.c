#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <sys/types.h>
#include <dirent.h>
#include "esp_log.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "chorder_display.h"
#include "freertos/task.h"
#include "config.h"

FontxFile fx16G[2],
 fx24G[2],
 fx32G[2],
 fx16M[2],
 fx24M[2],
 fx32M[2];

TFT_t dev;

lcd_style_t lcd_style = { 
  .background_color = BLACK,
  .foreground_color = RED,
  .alert_foreground_color = WHITE,
  .alert_background_color = RED,
  .success_foreground_color = PURPLE,
  .success_background_color = GREEN,
};

lcd_state_t lcd_state = { 
  .message = "",
  .alert = "",
  .success = "",
  .wifi_connected = false,
  .bluetooth_connected = false,
};

TickType_t display_timeout_last_activity = 0;

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

void clear_lcd(uint16_t color) {
  lcdFillScreen(&dev, color);
}

void render_message(FontxFile *fx, uint16_t color, int x_off, int y_off, int direction, unsigned char *message) {
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

  lcdSetFontDirection(&dev, direction);
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
    lcdDrawString(&dev, fx, 0, (line_number * fontHeight) + fontHeight-1, buf, color);
    line_number++;
  }
}

void render_display_task (void *pvParameters)
{
  static lcd_state_t last_rendered;
  static lcd_style_t last_style;
  static TickType_t last_popup_tick = 0;

  if (0 == display_timeout_last_activity)
    display_timeout_last_activity = xTaskGetTickCount();
  if (0 == last_popup_tick)
    last_popup_tick = xTaskGetTickCount();


  InitFontx(fx16G,"/spiffs/ILGH16XB.FNT",""); // 8x16Dot Gothic
  InitFontx(fx24G,"/spiffs/ILGH24XB.FNT",""); // 12x24Dot Gothic
  InitFontx(fx32G,"/spiffs/ILGH32XB.FNT",""); // 16x32Dot Gothic

  InitFontx(fx16M,"/spiffs/ILMH16XB.FNT",""); // 8x16Dot Mincyo
  InitFontx(fx24M,"/spiffs/ILMH24XB.FNT",""); // 12x24Dot Mincyo
  InitFontx(fx32M,"/spiffs/ILMH32XB.FNT",""); // 16x32Dot Mincyo

  while (1) {
    vTaskDelay(100 / portTICK_RATE_MS);


    // Compare with last display state to stop excessive blinking while re-rendering:
    // (This'd be best solved through double buffering, butehm...)
    if (0 != memcmp(&last_rendered,&lcd_state,sizeof(lcd_state_t))
        || 0 != memcmp(&last_style,&lcd_style,sizeof(lcd_style_t))) {
      lcdDisplayOn(&dev);
      lcdBacklightOn(&dev);
      display_timeout_last_activity = xTaskGetTickCount();
      if (0 != strlen(lcd_state.alert)) {
        clear_lcd(lcd_style.alert_background_color);
        render_message(fx24G,lcd_style.alert_foreground_color,0,20,DIR_W_TO_E,(unsigned char *)lcd_state.alert);
        last_popup_tick = xTaskGetTickCount();
      }  else {
        clear_lcd(lcd_style.background_color);
        // If there's a success message, let's have it:
        if (0 != strlen(lcd_state.success)) {
          lcdDrawFillRect(&dev, 0, 0, CONFIG_WIDTH, CONFIG_HEIGHT/2,lcd_style.success_background_color);
          lcdDrawFillRect(&dev, 0, 0, CONFIG_WIDTH, 16,lcd_style.success_background_color);
          render_message(fx16G,lcd_style.success_foreground_color,0,20,DIR_W_TO_E,(unsigned char *)lcd_state.success);
          last_popup_tick = xTaskGetTickCount();
        } else {
          render_message(fx16G,lcd_style.foreground_color,0,20,DIR_W_TO_E,(unsigned char *)lcd_state.message);
        }
      }

      // 2 is margin from screen edge:
      lcdDrawFillCircle(&dev, CONFIG_WIDTH-2-3, CONFIG_HEIGHT-2-3, 3, lcd_state.wifi_connected ? WHITE : RED);
      lcdDrawFillRect(&dev,
          CONFIG_WIDTH-(
            2+      // margin against edge of screen
            2*3+2+  // wifi circle plus margin
            6),      // size of rectangle
          CONFIG_HEIGHT-(
            2+       // margin against edge of screen
            6),      // size of rectangle
          CONFIG_WIDTH-(
            2+       // margin against edge of screen
            2*3+2),  // wifi circle plus margin
          CONFIG_HEIGHT-(
            2),       // margin against edge of screen
          lcd_state.bluetooth_connected ? BLUE : RED);

      memcpy(&last_rendered,&lcd_state,sizeof(lcd_state_t));
      memcpy(&last_style,&lcd_style,sizeof(lcd_style_t));
    }

    int ms_since_last_update = (xTaskGetTickCount()-display_timeout_last_activity)*portTICK_RATE_MS;
    int ms_since_last_popup = (xTaskGetTickCount()-last_popup_tick)*portTICK_RATE_MS;

    if (ms_since_last_popup > 5000) {
      strcpy(lcd_state.alert,"");
      strcpy(lcd_state.success,"");
    }
    if (ms_since_last_update < 30000) {
      lcdDisplayOn(&dev);
      lcdBacklightOn(&dev);
    } else {
      lcdDisplayOff(&dev);
      lcdBacklightOff(&dev);
    }
  }
}
