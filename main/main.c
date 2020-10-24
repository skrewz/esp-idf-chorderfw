#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"

#include "esp_err.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"

#include "esp_bt_defs.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_int_wdt.h"

#include "hid_dev.h"
#include "config.h"

#include "driver/gpio.h"
#include "st7789.h"
#include "fontx.h"
#include "bmpfile.h"
#include "decode_image.h"
#include "pngle.h"

#include "chordmappings.h"

#define	INTERVAL		400
#define WAIT	vTaskDelay(INTERVAL)

// GPIO 34-39 do not support pull-up/pull-down and are input-only
// input only        GPIO_NUM_36 
#define F_THUMB_PIN  GPIO_NUM_13
#define C_THUMB_PIN  GPIO_NUM_12
// input only        GPIO_NUM_39 
#define N_THUMB_PIN  GPIO_NUM_32
#define INDEX_PIN    GPIO_NUM_33
#define MIDDLE_PIN   GPIO_NUM_25
#define RING_PIN     GPIO_NUM_26
#define PINKY_PIN    GPIO_NUM_27
// GND pins by means of output 0. (Too lazy to solder a massive 1-to-7
// GND dupont wire. 🙃)
#define GND_PIN0 GPIO_NUM_21
#define GND_PIN1 GPIO_NUM_22
#define GND_PIN2 GPIO_NUM_17
#define GND_PIN3 GPIO_NUM_2
#define GND_PIN4 GPIO_NUM_15

static const char *TAG = "ST7789";


////////////////////////////////////////////////////////////////////////////////
// For keyboard functionality per se
////////////////////////////////////////////////////////////////////////////////
// {{{

enum State {
  PRESSING,
  RELEASING,
};


enum Mode {
  ALPHA,
  NUMSYM,
  FUNCTION
};

bool isCapsLocked = false;
bool isNumsymLocked = false;
keymap_t modKeys = 0x00;

enum Mode mode = ALPHA;

// }}}

////////////////////////////////////////////////////////////////////////////////
// Configuration relevant to bluetooth functionality
////////////////////////////////////////////////////////////////////////////////
// {{{
/** @warning Currently (07.2020) whitelisting devices is still not possible for all devices,
 * because many BT devices use resolvable random addresses, this seems to unsupported:
 * https://github.com/espressif/esp-idf/issues/1368
 * https://github.com/espressif/esp-idf/issues/2262
 * Therefore, if we enable pairing only on request, it is not possible to connect
 * to the ESP32 anymore. Either the ESP32 is visible by all devices or none.
 *
 * To circumvent any problems with this repository, if the config for
 * disabled pairing by default is active, we throw an error here.
 * @todo Check regularily for updates on the above mentioned issues.
 **/
#if CONFIG_MODULE_BT_PAIRING
#error "Sorry, currently the BT controller of the ESP32 does NOT support whitelisting. Please deactivate the pairing on demand option in make menuconfig!"
#endif

static uint16_t hid_conn_id = 0;
static bool sec_conn = false;
#define CHAR_DECLARATION_SIZE   (sizeof(uint8_t))

static void hidd_event_callback(esp_hidd_cb_event_t event, esp_hidd_cb_param_t *param);

#define MOUSE_SPEED 30
#define MAX_CMDLEN  100

#define EXT_UART_TAG "EXT_UART"
#define CONSOLE_UART_TAG "CONSOLE_UART"

static config_data_t config;

#define CMDSTATE_IDLE 0
#define CMDSTATE_GET_RAW 1
#define CMDSTATE_GET_ASCII 2

struct cmdBuf {
    int state;
    int expectedBytes;
    int bufferLength;
    uint8_t buf[MAX_CMDLEN];
};
static uint8_t manufacturer[14]= {'Y','o','u','r',' ','n','a','m','e',' ','h','e','r','e'};

static uint8_t hidd_service_uuid128[] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x12, 0x18, 0x00, 0x00,
};

/** @brief Event bit, set if pairing is enabled
 * @note If MODULE_BT_PAIRING ist set in menuconfig, this bit is disable by default
 * and can be enabled via $PM1 , disabled via $PM0.
 * If MODULE_BT_PAIRING is not set, this bit will be set on boot.*/
#define SYSTEM_PAIRING_ENABLED (1<<0)

/** @brief Event bit, set if the ESP32 is currently advertising.
 *
 * Used for determining if we need to set advertising params again,
 * when the pairing mode is changed. */
#define SYSTEM_CURRENTLY_ADVERTISING (1<<1)
// }}}

////////////////////////////////////////////////////////////////////////////////
// Configuration relevant to ST7789:
////////////////////////////////////////////////////////////////////////////////
// {{{

static void SPIFFS_Directory(char * path) {
    DIR* dir = opendir(path);
    assert(dir != NULL);
    while (true) {
        struct dirent*pe = readdir(dir);
        if (!pe) break;
        ESP_LOGI(__FUNCTION__,"d_name=%s d_ino=%d d_type=%x", pe->d_name,pe->d_ino, pe->d_type);
    }
    closedir(dir);
}

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

// }}}

////////////////////////////////////////////////////////////////////////////////
// Helper functions for bluetooth behaviour
////////////////////////////////////////////////////////////////////////////////
// {{{


/** @brief Event group for system status */
EventGroupHandle_t eventgroup_system;

static esp_ble_adv_data_t hidd_adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x000A, //slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval = 0x0010, //slave connection max interval, Time = max_interval * 1.25 msec
    .appearance = 0x03c0,       //HID Generic,
    .manufacturer_len = 0,
    .p_manufacturer_data =  NULL,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(hidd_service_uuid128),
    .p_service_uuid = hidd_service_uuid128,
    .flag = 0x6,
};

// config scan response data
///@todo Scan response is currently not used. If used, add state handling (adv start) according to ble/gatt_security_server example of Espressif
static esp_ble_adv_data_t hidd_adv_resp = {
    .set_scan_rsp = true,
    .include_name = true,
    .manufacturer_len = sizeof(manufacturer),
    .p_manufacturer_data = manufacturer,
};

static esp_ble_adv_params_t hidd_adv_params = {
    .adv_int_min        = 0x20,
    .adv_int_max        = 0x30,
    .adv_type           = ADV_TYPE_IND,
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    //.peer_addr            =
    //.peer_addr_type       =
    .channel_map        = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static void hidd_event_callback(esp_hidd_cb_event_t event, esp_hidd_cb_param_t *param)
{
    switch(event) {
        case ESP_HIDD_EVENT_REG_FINISH: {
                                            if (param->init_finish.state == ESP_HIDD_INIT_OK) {
                                                //esp_bd_addr_t rand_addr = {0x04,0x11,0x11,0x11,0x11,0x05};
                                                esp_ble_gap_set_device_name(config.bt_device_name);
                                                esp_ble_gap_config_adv_data(&hidd_adv_data);
                                            }
                                            break;
                                        }
        case ESP_BAT_EVENT_REG: {
                                    break;
                                }
        case ESP_HIDD_EVENT_DEINIT_FINISH:
                                break;
        case ESP_HIDD_EVENT_BLE_CONNECT: {
                                             ESP_LOGI(TAG, "ESP_HIDD_EVENT_BLE_CONNECT");
                                             hid_conn_id = param->connect.conn_id;
                                             xEventGroupClearBits(eventgroup_system,SYSTEM_CURRENTLY_ADVERTISING);
                                             break;
                                         }
        case ESP_HIDD_EVENT_BLE_DISCONNECT: {
                                                sec_conn = false;
                                                ESP_LOGI(TAG, "ESP_HIDD_EVENT_BLE_DISCONNECT");
                                                esp_ble_gap_start_advertising(&hidd_adv_params);
                                                xEventGroupSetBits(eventgroup_system,SYSTEM_CURRENTLY_ADVERTISING);
                                                break;
                                            }
        case ESP_HIDD_EVENT_BLE_VENDOR_REPORT_WRITE_EVT: {
                                                             ESP_LOGI(TAG, "%s, ESP_HIDD_EVENT_BLE_VENDOR_REPORT_WRITE_EVT", __func__);
                                                             ESP_LOG_BUFFER_HEX(TAG, param->vendor_write.data, param->vendor_write.length);
                                                         }
        case ESP_HIDD_EVENT_BLE_LED_OUT_WRITE_EVT: {
                                                       ESP_LOGI(TAG, "%s, ESP_HIDD_EVENT_BLE_LED_OUT_WRITE_EVT, keyboard LED value: %d", __func__, param->vendor_write.data[0]);
                                                   }
        default:
                                                   break;
    }
    return;
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            esp_ble_gap_start_advertising(&hidd_adv_params);
            xEventGroupSetBits(eventgroup_system,SYSTEM_CURRENTLY_ADVERTISING);
            break;
        case ESP_GAP_BLE_SEC_REQ_EVT:
            for(int i = 0; i < ESP_BD_ADDR_LEN; i++) {
                ESP_LOGD(TAG, "%x:",param->ble_security.ble_req.bd_addr[i]);
            }
            esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
            break;
        case ESP_GAP_BLE_AUTH_CMPL_EVT:
            sec_conn = true;
            esp_bd_addr_t bd_addr;
            memcpy(bd_addr, param->ble_security.auth_cmpl.bd_addr, sizeof(esp_bd_addr_t));
            ESP_LOGI(TAG, "remote BD_ADDR: %08x%04x",\
                    (bd_addr[0] << 24) + (bd_addr[1] << 16) + (bd_addr[2] << 8) + bd_addr[3],
                    (bd_addr[4] << 8) + bd_addr[5]);
            ESP_LOGI(TAG, "address type = %d", param->ble_security.auth_cmpl.addr_type);
            ESP_LOGI(TAG, "pair status = %s",param->ble_security.auth_cmpl.success ? "success" : "fail");
            if(!param->ble_security.auth_cmpl.success) {
                ESP_LOGE(TAG, "fail reason = 0x%x",param->ble_security.auth_cmpl.fail_reason);
            } else {
                xEventGroupClearBits(eventgroup_system,SYSTEM_CURRENTLY_ADVERTISING);
            }
#if CONFIG_MODULE_BT_PAIRING
            //add connected device to whitelist (necessary if whitelist connections only).
            if(esp_ble_gap_update_whitelist(true,bd_addr,BLE_WL_ADDR_TYPE_PUBLIC) != ESP_OK)
            {
                ESP_LOGW(TAG,"cannot add device to whitelist, with public address");
            } else {
                ESP_LOGI(TAG,"added device to whitelist");
            }
            if(esp_ble_gap_update_whitelist(true,bd_addr,BLE_WL_ADDR_TYPE_RANDOM) != ESP_OK)
            {
                ESP_LOGW(TAG,"cannot add device to whitelist, with random address");
            }
#endif
            break;
        default:
            break;
    }
}

// }}}

////////////////////////////////////////////////////////////////////////////////
// Implementing chorder GPIO functionality
////////////////////////////////////////////////////////////////////////////////
// {{{
void set_up_input_pin (int pinnum)
{
    gpio_reset_pin(pinnum);
    if (ESP_OK != gpio_set_direction(pinnum, GPIO_MODE_INPUT))
    {
        ESP_LOGE(__FUNCTION__,"Failure setting pin %d's direction", pinnum);
    }
    if (ESP_OK != gpio_set_pull_mode(pinnum, GPIO_PULLUP_ONLY))
    {
        ESP_LOGE(__FUNCTION__,"Failure setting pin %d's to pull-up", pinnum);
    }
}
void set_up_gnd_pin (int pinnum)
{
    gpio_reset_pin(pinnum);
    if (ESP_OK != gpio_set_direction(pinnum, GPIO_MODE_OUTPUT))
    {
        ESP_LOGE(__FUNCTION__,"Failure setting pin %d's direction", pinnum);
    }
    if (ESP_OK != gpio_set_level(pinnum, 0))
    {
        ESP_LOGE(__FUNCTION__,"Failure pin %d to level 0", pinnum);
    }
}
uint8_t get_current_state ()
{
    uint8_t state = 0;
    state |= (!gpio_get_level(F_THUMB_PIN) << 6);
    state |= (!gpio_get_level(C_THUMB_PIN) << 5);
    state |= (!gpio_get_level(N_THUMB_PIN) << 4);
    state |= (!gpio_get_level(INDEX_PIN)   << 3);
    state |= (!gpio_get_level(MIDDLE_PIN)  << 2);
    state |= (!gpio_get_level(RING_PIN)    << 1);
    state |= (!gpio_get_level(PINKY_PIN)   << 0);
    return state;
}

void sendRawKey(uint8_t modKey, uint8_t rawKey){
  uint8_t buf[8];
  memset(buf,0x00,8);
  buf[0] = rawKey;
  esp_hidd_send_keyboard_value(hid_conn_id,modKey,buf,1);
  ESP_LOGI("sendRawKey","esp_hidd_send_keyboard_value(%d,0x%x,0x%x %x %x %x %x %x %x %x)",
      hid_conn_id,modKey,
      buf[0],buf[1],buf[2],buf[3],
      buf[4],buf[5],buf[6],buf[7]);
  buf[0] = 0x00;
  esp_hidd_send_keyboard_value(hid_conn_id,modKey,buf,1);
}

void sendControlKey(char *cntrlName){
  // note: for Volume +/- and the few other keys that take a time to hold, simply add it into the string
  // for example:
  //    sendControlKey("VOLUME+,500")
  // will send Volume up and hold it for half a second
  //ble.print("AT+BLEHIDCONTROLKEY=");
  //ble.println(cntrlName);  
}  

void sendKey(uint8_t keyState){
  keymap_t theKey;  
  // Determine the key based on the current mode's keymap
  if (mode == ALPHA) {
    theKey = keymap_default[keyState];
  } else if (mode == NUMSYM) {
    theKey = keymap_numsym[keyState];
  } else {
    theKey = keymap_function[keyState];
  }

  ESP_LOGI(TAG, "Have theKey=%x", theKey);

  switch (theKey)  {
  // Handle mode switching - return immediately after the mode has changed
  // Handle basic mode switching
  case MODE_NUM:
    if (mode == NUMSYM) {
      mode = ALPHA;
    } else {
      mode = NUMSYM;
    }
    return;
  case MODE_FUNC:
    if (mode == FUNCTION) {
      mode = ALPHA;
    } else {
      mode = FUNCTION;
    }
    return;
  case MODE_RESET:
    mode = ALPHA;
    modKeys = 0x00;
    isCapsLocked = false;
    isNumsymLocked = false;
    return;
  case MODE_MRESET:
    mode = ALPHA;
    modKeys = 0x00;
    isCapsLocked = false;
    isNumsymLocked = false;       
    //digitalWrite(EnPin, LOW);  // turn off 3.3v regulator enable.
    return;
  // Handle mode locks
  case HID_KEY_CAPS_LOCK:
    if (isCapsLocked){
      isCapsLocked = false;
      modKeys = 0x00;
    } else {
      isCapsLocked = true;
      modKeys = 0x02;
    }
    return;
  case MODE_NUMLCK:
    if (isNumsymLocked){
      isNumsymLocked = false;
      mode = ALPHA;
    } else {
      isNumsymLocked = true;
      mode = NUMSYM;
    }
    return;
  // Handle modifier keys toggling
  case MOD_LCTRL:
    modKeys = modKeys ^ 0x01;
    return;
  case MOD_LSHIFT:
    modKeys = modKeys ^ 0x02;
    return;
  case MOD_LALT:
    modKeys = modKeys ^ 0x04;
    return;
  case MOD_LGUI:
    modKeys = modKeys ^ 0x08;
    return;
  case MOD_RCTRL:
    modKeys = modKeys ^ 0x10;
    return;
  case MOD_RSHIFT:
    modKeys = modKeys ^ 0x20;
    return;
  case MOD_RALT:
    modKeys = modKeys ^ 0x40;
    return;
  case MOD_RGUI:
    modKeys = modKeys ^ 0x80;
    return;
  // Handle special keys
  case MULTI_NumShift:
    if (mode == NUMSYM) {
      mode = ALPHA;
    } else {
      mode = NUMSYM;
    }
    modKeys = modKeys ^ 0x02;
    return;
  case MULTI_CtlAlt:
    modKeys = modKeys ^ 0x01;
    modKeys = modKeys ^ 0x04;
    return;
  /* Everything after this sends actual keys to the system; break rather than
     return since we want to reset the modifiers after these keys are sent. */
  case MACRO_000:
    sendRawKey(0x00, 0x27);
    sendRawKey(0x00, 0x27);
    sendRawKey(0x00, 0x27);
    break;
  case MACRO_00:
    sendRawKey(0x00, 0x27);
    sendRawKey(0x00, 0x27);
    break;
  case MACRO_quotes:
    sendRawKey(0x02, 0x34);
    sendRawKey(0x02, 0x34);
    sendRawKey(0x00, 0x50);
    break;
  case MACRO_parens:
    sendRawKey(0x02, 0x26);
    sendRawKey(0x02, 0x27);
    sendRawKey(0x00, 0x50);
    break;
  case MACRO_dollar:
    sendRawKey(0x02, 0x21);
    break;
  case MACRO_percent:
    sendRawKey(0x02, 0x22);
    break;
  case MACRO_ampersand:
    sendRawKey(0x02, 0x24);
    break;
  case MACRO_asterisk:
    sendRawKey(0x02, 0x25);
    break;
  case MACRO_question:
    sendRawKey(0x02, 0x38);
    break;
  case MACRO_plus:
    sendRawKey(0x02, 0x2E);
    break;
  case MACRO_openparen:
    sendRawKey(0x02, 0x26);
    break;
  case MACRO_closeparen:
    sendRawKey(0x02, HID_KEY_0);
    break;
  case MACRO_opencurly:
    sendRawKey(0x02, 0x2F);
    break;
  case MACRO_closecurly:
    sendRawKey(0x02, 0x30);
    break;
  // Handle Android specific keys
  case ANDROID_search:
    sendRawKey(0x04, 0x2C);
    break;
  case ANDROID_home:
    sendRawKey(0x04, 0x29);
    break;
  case ANDROID_menu:
    sendRawKey(0x10, 0x29);
    break;
  case ANDROID_back:
    sendRawKey(0x00, 0x29);
    break;
  case ANDROID_dpadcenter:
    sendRawKey(0x00, 0x5D);
    break;
  case MEDIA_playpause:
    sendControlKey("PLAYPAUSE");
    break;
  case MEDIA_stop:
    sendControlKey("MEDIASTOP");
    break;
  case MEDIA_next:
    sendControlKey("MEDIANEXT");
    break;
  case MEDIA_previous:
    sendControlKey("MEDIAPREVIOUS");
    break;
  case MEDIA_volup:
    sendControlKey("VOLUME+,500");
    break;
  case MEDIA_voldn:
    sendControlKey("VOLUME-,500");
    break;
  // Send the key
  default:
    sendRawKey(modKeys, theKey);
    break;
  }

  modKeys = 0x00;
  mode = ALPHA;
  // Reset the modKeys and mode based on locks
  if (isCapsLocked){
    modKeys = 0x02;
  }
  if (isNumsymLocked){
    mode = NUMSYM;
  }
}
// }}}

////////////////////////////////////////////////////////////////////////////////
// Implementing chorder display functionality
////////////////////////////////////////////////////////////////////////////////
// {{{
TickType_t DirectionTest(TFT_t * dev, FontxFile *fx, int width, int height) {
    TickType_t startTick, endTick, diffTick;
    startTick = xTaskGetTickCount();

    // get font width & height
    uint8_t buffer[FontxGlyphBufSize];
    uint8_t fontWidth;
    uint8_t fontHeight;
    GetFontx(fx, 0, buffer, &fontWidth, &fontHeight);
    //ESP_LOGI(__FUNCTION__,"fontWidth=%d fontHeight=%d",fontWidth,fontHeight);

    uint16_t color;
    lcdFillScreen(dev, BLACK);
    uint8_t ascii[20];

    color = RED;
    strcpy((char *)ascii, "Direction=0");
    lcdSetFontDirection(dev, 0);
    lcdDrawString(dev, fx, 0, fontHeight-1, ascii, color);

    color = BLUE;
    strcpy((char *)ascii, "Direction=2");
    lcdSetFontDirection(dev, 2);
    lcdDrawString(dev, fx, (width-1), (height-1)-(fontHeight*1), ascii, color);

    color = CYAN;
    strcpy((char *)ascii, "Direction=1");
    lcdSetFontDirection(dev, 1);
    lcdDrawString(dev, fx, (width-1)-fontHeight, 0, ascii, color);

    color = GREEN;
    strcpy((char *)ascii, "Direction=3");
    lcdSetFontDirection(dev, 3);
    lcdDrawString(dev, fx, (fontHeight-1), height-1, ascii, color);

    endTick = xTaskGetTickCount();
    diffTick = endTick - startTick;
    ESP_LOGI(__FUNCTION__, "elapsed time[ms]:%d",diffTick*portTICK_RATE_MS);
    return diffTick;
}


TickType_t FillRectTest(TFT_t * dev, int width, int height) {
    TickType_t startTick, endTick, diffTick;
    startTick = xTaskGetTickCount();

    uint16_t color;
    lcdFillScreen(dev, CYAN);

    uint16_t red;
    uint16_t green;
    uint16_t blue;
    srand( (unsigned int)time( NULL ) );
    for(int i=1;i<100;i++) {
        red=rand()%255;
        green=rand()%255;
        blue=rand()%255;
        color=rgb565_conv(red, green, blue);
        uint16_t xpos=rand()%width;
        uint16_t ypos=rand()%height;
        uint16_t size=rand()%(width/5);
        lcdDrawFillRect(dev, xpos, ypos, xpos+size, ypos+size, color);
    }

    endTick = xTaskGetTickCount();
    diffTick = endTick - startTick;
    ESP_LOGI(__FUNCTION__, "elapsed time[ms]:%d",diffTick*portTICK_RATE_MS);
    return diffTick;
}

void ST7789(void *pvParameters)
{
    // set font file
    FontxFile fx16G[2];
    FontxFile fx24G[2];
    FontxFile fx32G[2];
    InitFontx(fx16G,"/spiffs/ILGH16XB.FNT",""); // 8x16Dot Gothic
    InitFontx(fx24G,"/spiffs/ILGH24XB.FNT",""); // 12x24Dot Gothic
    InitFontx(fx32G,"/spiffs/ILGH32XB.FNT",""); // 16x32Dot Gothic

    FontxFile fx16M[2];
    FontxFile fx24M[2];
    FontxFile fx32M[2];
    InitFontx(fx16M,"/spiffs/ILMH16XB.FNT",""); // 8x16Dot Mincyo
    InitFontx(fx24M,"/spiffs/ILMH24XB.FNT",""); // 12x24Dot Mincyo
    InitFontx(fx32M,"/spiffs/ILMH32XB.FNT",""); // 16x32Dot Mincyo

    TFT_t dev;
    spi_master_init(&dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO, CONFIG_BL_GPIO);
    lcdInit(&dev, CONFIG_WIDTH, CONFIG_HEIGHT, CONFIG_OFFSETX, CONFIG_OFFSETY);

#if CONFIG_INVERSION
    ESP_LOGI(TAG, "Enable Display Inversion");
    lcdInversionOn(&dev);
#endif

    while(1) {
        if (CONFIG_WIDTH >= 240) {
            DirectionTest(&dev, fx24G, CONFIG_WIDTH, CONFIG_HEIGHT);
        } else {
            DirectionTest(&dev, fx16G, CONFIG_WIDTH, CONFIG_HEIGHT);
        }
        WAIT;

        // random squares
        FillRectTest(&dev, CONFIG_WIDTH, CONFIG_HEIGHT);
        WAIT;

        lcdDisplayOff(&dev);
        lcdBacklightOff(&dev);
        WAIT;
        lcdDisplayOn(&dev);
        lcdBacklightOn(&dev);

    } // end while

    // never reach
    while (1) {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}
// }}}

////////////////////////////////////////////////////////////////////////////////
// Main FreeRTOS tasks
////////////////////////////////////////////////////////////////////////////////
// {{{
void watch_for_key_changes (void *pvParameters)
{
    uint8_t kbdcmd[] = {0x00,0x00};
    uint8_t lastKeyState = 0;
    bool have_seen_first_stable_reading = false;

    int64_t lastDebounceTime = 0;  // the last time inputs changed
    int64_t debounceDelay = 10000; // the debounce time; increase if the output flickers

    uint8_t previousStableReading = 0;
    uint8_t currentStableReading = 0;

    enum State state = RELEASING;

    set_up_gnd_pin(GND_PIN0);
    set_up_gnd_pin(GND_PIN1);
    set_up_gnd_pin(GND_PIN2);
    set_up_gnd_pin(GND_PIN3);
    set_up_gnd_pin(GND_PIN4);
    set_up_input_pin(F_THUMB_PIN);
    set_up_input_pin(C_THUMB_PIN);
    set_up_input_pin(N_THUMB_PIN);
    set_up_input_pin(INDEX_PIN);
    set_up_input_pin(MIDDLE_PIN);
    set_up_input_pin(RING_PIN);
    set_up_input_pin(PINKY_PIN);

    while (1) {
        // Build the current key state.
        uint8_t keyState = get_current_state();

        if (lastKeyState != keyState) {
            lastDebounceTime = esp_timer_get_time();
        }

        if ((esp_timer_get_time() - lastDebounceTime) > debounceDelay) {
            // whatever the reading is at, it's been there for longer
            // than the debounce delay, so take it as the actual current state:
            currentStableReading = keyState;
        }

        if (previousStableReading != currentStableReading) {
            //Serial.print(F("currentStableReading now "));
            //Serial.println(currentStableReading);
            ESP_LOGI(TAG, "New reading: %s%s%s %s%s%s%s",
                currentStableReading & (1 << 6) ? "F" : "_",
                currentStableReading & (1 << 5) ? "C" : "_",
                currentStableReading & (1 << 4) ? "N" : "_",
                currentStableReading & (1 << 3) ? "I" : "_",
                currentStableReading & (1 << 2) ? "M" : "_",
                currentStableReading & (1 << 1) ? "R" : "_",
                currentStableReading & (1 << 0) ? "P" : "_"
            );
            if (! have_seen_first_stable_reading)
            {
            /*
                bool are_thumbs_down = 0x70 == currentStableReading;
                if (are_thumbs_down)
                {
                    ESP_LOGI(TAG, "Performing a BT factory reset: ");
                    if ( ! ble.factoryReset() ){
                        ESP_LOGW(TAG, "Factory reset failed!");
                    }
                    ESP_LOGI(TAG, "Resetting Arduino...");
                    resetFunc();
                }
            */
            }
            have_seen_first_stable_reading = true;
            switch (state) {
              case PRESSING:
                if (previousStableReading & ~currentStableReading) {
                  state = RELEASING;
                  sendKey(previousStableReading);
                } 
                break;

              case RELEASING:
                if (currentStableReading & ~previousStableReading) {
                  state = PRESSING;
                }
                break;
            }
            previousStableReading = currentStableReading;
        }
        lastKeyState = keyState;
        vTaskDelay(10 / portTICK_RATE_MS);
    }
}

void BLE_muckery(void *pvParameters)
{
    while(1) {
        if (sec_conn) esp_hidd_send_mouse_value(hid_conn_id,0,-MOUSE_SPEED,0,0);
        vTaskDelay(50);
        if (sec_conn) esp_hidd_send_mouse_value(hid_conn_id,0,0,MOUSE_SPEED,0);
        vTaskDelay(50);
        if (sec_conn) esp_hidd_send_mouse_value(hid_conn_id,0,MOUSE_SPEED,0,0);
        vTaskDelay(50);
        if (sec_conn) esp_hidd_send_mouse_value(hid_conn_id,0,0,-MOUSE_SPEED,0);
        vTaskDelay(50);
    }
}
// }}}


void app_main(void)
{
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

    // Initialize FreeRTOS elements
    eventgroup_system = xEventGroupCreate();
    if(eventgroup_system == NULL) ESP_LOGE(TAG, "Cannot initialize event group");
    //if set in KConfig, pairing is disable by default.
    //User has to enable pairing with $PM1
#if CONFIG_MODULE_BT_PAIRING
    ESP_LOGI(TAG,"pairing disabled by default");
    xEventGroupClearBits(eventgroup_system,SYSTEM_PAIRING_ENABLED);
    hidd_adv_params.adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_WLST;
#else
    ESP_LOGI(TAG,"pairing enabled by default");
    xEventGroupSetBits(eventgroup_system,SYSTEM_PAIRING_ENABLED);
    hidd_adv_params.adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY;
#endif

    // Initialize NVS.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "%s initialize controller failed\n", __func__);
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(TAG, "%s enable controller failed\n", __func__);
        return;
    }

    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(TAG, "%s init bluedroid failed\n", __func__);
        return;
    }

    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(TAG, "%s init bluedroid failed\n", __func__);
        return;
    }

    if((ret = esp_hidd_profile_init()) != ESP_OK) {
        ESP_LOGE(TAG, "%s init bluedroid failed\n", __func__);
    }

    // Read config
    nvs_handle my_handle;
    ESP_LOGI("MAIN","loading configuration from NVS");
    ret = nvs_open("config_c", NVS_READWRITE, &my_handle);
    if(ret != ESP_OK) ESP_LOGE("MAIN","error opening NVS");
    size_t available_size = MAX_BT_DEVICENAME_LENGTH;
    strcpy(config.bt_device_name, GATTS_TAG);
    nvs_get_str (my_handle, "btname", config.bt_device_name, &available_size);
    if(ret != ESP_OK)
    {
        ESP_LOGI("MAIN","error reading NVS - bt name, setting to default");
        strcpy(config.bt_device_name, GATTS_TAG);
    } else ESP_LOGI("MAIN","bt device name is: %s",config.bt_device_name);

    ret = nvs_get_u8(my_handle, "locale", &config.locale);
    //if(ret != ESP_OK || config.locale >= LAYOUT_MAX)
    ///@todo implement keyboard layouts.
    if(ret != ESP_OK)
    {
        ESP_LOGI("MAIN","error reading NVS - locale, setting to US_INTERNATIONAL");
        //config.locale = LAYOUT_US_INTERNATIONAL;
    } else ESP_LOGI("MAIN","locale code is : %d",config.locale);
    nvs_close(my_handle);
    ///@todo How to handle the locale here? We have the memory for full lookups on the ESP32, but how to communicate this with the Teensy?

    ///register the callback function to the gap module
    esp_ble_gap_register_callback(gap_event_handler);
    esp_hidd_register_callbacks(hidd_event_callback);

    /* set the security iocap & auth_req & key size & init key response key parameters to the stack*/
    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_BOND;     //bonding with peer device after authentication
    esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;           //set the IO capability to No output No input
    uint8_t key_size = 16;      //the key size should be 7~16 bytes
    uint8_t init_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
    esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));
    /* If your BLE device act as a Slave, the init_key means you hope which types of key of the master should distribute to you,
       and the response key means which key you can distribute to the Master;
       If your BLE device act as a master, the response key means you hope which types of key of the slave should distribute to you,
       and the init key means which key you can distribute to the slave. */
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_INIT_KEY, &init_key, sizeof(uint8_t));
    esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));

    xTaskCreate(ST7789, "ST7789", 1024*6, NULL, 2, NULL);
    //xTaskCreate(BLE_muckery, "BLE_muckery", 1024*6, NULL, 2, NULL);
    xTaskCreate(watch_for_key_changes, "watch_for_key_changes", 1024*3, NULL, 2, NULL);
}
