#include "st7789.h"
#include <stdint.h>
#include "config.h"

#define DIR_W_TO_E 0
#define DIR_N_TO_S 1
#define DIR_E_TO_W 2
#define DIR_S_TO_N 3

// You have to set these CONFIG value using menuconfig.
#if 0
#define CONFIG_WIDTH  240
#define CONFIG_HEIGHT 240
#define CONFIG_MOSI_GPIO 23
#define CONFIG_SCLK_GPIO 18
#define CONFIG_CS_GPIO -1
#define CONFIG_DC_GPIO 19
#define CONFIG_RESET_GPIO 15
#define CONFIG_BL_GPIO -1
#endif

void SPIFFS_Directory(char * path);
void initialize_lcd();
void clear_lcd(TFT_t * dev, uint16_t color);
void render_message(
    TFT_t * dev, 
    FontxFile *fx, 
    uint16_t color, 
    int x_off, 
    int y_off, 
    int direction, 
    unsigned char *message);


typedef struct {
  uint16_t background_color;
  uint16_t foreground_color;
  uint16_t alert_foreground_color;
  uint16_t alert_background_color;
} lcd_style_t;

typedef struct {
  char message[INTERNAL_BUFSIZE];
  char alert[INTERNAL_BUFSIZE];
} lcd_state_t;

