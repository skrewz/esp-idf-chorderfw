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

#include "esp_bt_defs.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_gatt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_int_wdt.h"
#include "esp_tls.h"
#include "esp_sleep.h"
#include "esp_http_client.h"

#include "hid_dev.h"
#include "config.h"

#include "chorder_display.h"
#include "chorder_wifi.h"

#include "driver/gpio.h"
#include "driver/rtc_io.h"
#include "fontx.h"
#include "bmpfile.h"
#include "decode_image.h"
#include "pngle.h"

#include "chordmappings.h"



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

enum Operating_mode {
  OPMODE_NOTETAKING,
  OPMODE_BLE_KEYBOARD,
  OPMODE_BLE_MOUSE,
};

// the current function that'll take keystate updates:
// This receives a shifted-into-place bit string of the latest key press
void (*keystate_handler)(uint8_t keyState);


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


TFT_t dev;
FontxFile fx16G[2],
 fx24G[2],
 fx32G[2],
 fx16M[2],
 fx24M[2],
 fx32M[2];

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

//static uint8_t manufacturer[14]= {'Y','o','u','r',' ','n','a','m','e',' ','h','e','r','e'};
// config scan response data
///@todo Scan response is currently not used. If used, add state handling (adv start) according to ble/gatt_security_server example of Espressif
//static esp_ble_adv_data_t hidd_adv_resp = {
//    .set_scan_rsp = true,
//    .include_name = true,
//    .manufacturer_len = sizeof(manufacturer),
//    .p_manufacturer_data = manufacturer,
//};

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
                                             ESP_LOGI(__FUNCTION__, "ESP_HIDD_EVENT_BLE_CONNECT");
                                             hid_conn_id = param->connect.conn_id;
                                             xEventGroupClearBits(eventgroup_system,SYSTEM_CURRENTLY_ADVERTISING);
                                             break;
                                         }
        case ESP_HIDD_EVENT_BLE_DISCONNECT: {
                                                sec_conn = false;
                                                ESP_LOGI(__FUNCTION__, "ESP_HIDD_EVENT_BLE_DISCONNECT");
                                                esp_ble_gap_start_advertising(&hidd_adv_params);
                                                xEventGroupSetBits(eventgroup_system,SYSTEM_CURRENTLY_ADVERTISING);
                                                break;
                                            }
        case ESP_HIDD_EVENT_BLE_VENDOR_REPORT_WRITE_EVT: {
                                                             ESP_LOGI(__FUNCTION__, "%s, ESP_HIDD_EVENT_BLE_VENDOR_REPORT_WRITE_EVT", __func__);
                                                             ESP_LOG_BUFFER_HEX(__FUNCTION__, param->vendor_write.data, param->vendor_write.length);
                                                             break;
                                                         }
        case ESP_HIDD_EVENT_BLE_LED_OUT_WRITE_EVT: {
                                                       ESP_LOGI(__FUNCTION__, "%s, ESP_HIDD_EVENT_BLE_LED_OUT_WRITE_EVT, keyboard LED value: %d", __func__, param->vendor_write.data[0]);
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
                ESP_LOGD(__FUNCTION__, "%x:",param->ble_security.ble_req.bd_addr[i]);
            }
            esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
            break;
        case ESP_GAP_BLE_AUTH_CMPL_EVT:
            sec_conn = true;
            esp_bd_addr_t bd_addr;
            memcpy(bd_addr, param->ble_security.auth_cmpl.bd_addr, sizeof(esp_bd_addr_t));
            ESP_LOGI(__FUNCTION__, "remote BD_ADDR: %08x%04x",\
                    (bd_addr[0] << 24) + (bd_addr[1] << 16) + (bd_addr[2] << 8) + bd_addr[3],
                    (bd_addr[4] << 8) + bd_addr[5]);
            ESP_LOGI(__FUNCTION__, "address type = %d", param->ble_security.auth_cmpl.addr_type);
            ESP_LOGI(__FUNCTION__, "pair status = %s",param->ble_security.auth_cmpl.success ? "success" : "fail");
            if(!param->ble_security.auth_cmpl.success) {
                ESP_LOGE(__FUNCTION__, "fail reason = 0x%x",param->ble_security.auth_cmpl.fail_reason);
            } else {
                xEventGroupClearBits(eventgroup_system,SYSTEM_CURRENTLY_ADVERTISING);
            }
#if CONFIG_MODULE_BT_PAIRING
            //add connected device to whitelist (necessary if whitelist connections only).
            if(esp_ble_gap_update_whitelist(true,bd_addr,BLE_WL_ADDR_TYPE_PUBLIC) != ESP_OK)
            {
                ESP_LOGW(__FUNCTION__,"cannot add device to whitelist, with public address");
            } else {
                ESP_LOGI(__FUNCTION__,"added device to whitelist");
            }
            if(esp_ble_gap_update_whitelist(true,bd_addr,BLE_WL_ADDR_TYPE_RANDOM) != ESP_OK)
            {
                ESP_LOGW(__FUNCTION__,"cannot add device to whitelist, with random address");
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

void send_chorder_to_sleep (void)
{
  gpio_reset_pin(C_THUMB_PIN);
  if (ESP_OK != rtc_gpio_init(C_THUMB_PIN))
  {
    ESP_LOGE(__FUNCTION__,"Failure initialising pin %d's = (%d) as RTC", C_THUMB_PIN, rtc_io_number_get(C_THUMB_PIN));
  }
  // Handle sleep mode stuff:
  if (ESP_OK != rtc_gpio_wakeup_enable(C_THUMB_PIN, GPIO_INTR_LOW_LEVEL))
  {
    ESP_LOGE(__FUNCTION__,"Failure setting pin %d's = (%d) RTC wakeup", C_THUMB_PIN, rtc_io_number_get(C_THUMB_PIN));
  }
  if (ESP_OK != rtc_gpio_pullup_en(C_THUMB_PIN))
  {
    ESP_LOGE(__FUNCTION__,"Failure setting pin %d's to pull-up", C_THUMB_PIN);
  }

  if (ESP_OK != rtc_gpio_pullup_en(rtc_io_number_get(C_THUMB_PIN))) {
    ESP_LOGE(__FUNCTION__,"Failure enabling RTC pullup");
  }
  if (ESP_OK != esp_sleep_enable_ext0_wakeup(C_THUMB_PIN, 0))
  {
    ESP_LOGE(__FUNCTION__,"Failure setting pin %d's esp_sleep_enable_ext0_wakeup", C_THUMB_PIN);
  }
  esp_deep_sleep_start();
}

bool urlencode_into(char *dest, size_t max_dest, const char *unescaped)
{
  char *outpos = dest;
  for(int i=0;i<strlen(unescaped);i++)
  {
    char addition[4];
    if (
        ('A' <= unescaped[i] && unescaped[i] <= 'Z') ||
        ('a' <= unescaped[i] && unescaped[i] <= 'z') ||
        ('0' <= unescaped[i] && unescaped[i] <= '9') ||
        '-' == unescaped[i] ||
        '_' == unescaped[i] ||
        '.' == unescaped[i] ||
        '~' == unescaped[i])
    {
      sprintf(addition,"%c",unescaped[i]);
    } else {
      sprintf(addition,"%%%02x",unescaped[i]);
    }
    int written = snprintf(outpos,max_dest-(outpos-dest),"%s",addition);
    if (written != strlen(addition))
      return false;
    outpos += written;
  }
  return true;
}


bool send_off_note(char *note)
{
  // Starting point:
  // https://github.com/espressif/esp-idf/blob/357a2776032299b8bc4044900a8f1d6950d7ce89/examples/protocols/esp_http_client/main/esp_http_client_example.c
  if (! wifi_is_connected())
    return false;

  char encoded[2*INTERNAL_BUFSIZE];

  esp_err_t _http_event_handle(esp_http_client_event_t *evt)
  {
    switch(evt->event_id) {
      case HTTP_EVENT_ERROR:
        ESP_LOGI(__FUNCTION__, "HTTP_EVENT_ERROR");
        break;
      case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(__FUNCTION__, "HTTP_EVENT_ON_CONNECTED");
        break;
      case HTTP_EVENT_HEADERS_SENT:
        ESP_LOGI(__FUNCTION__, "HTTP_EVENT_HEADERS_SENT");
        break;
      case HTTP_EVENT_ON_HEADER:
        ESP_LOGI(__FUNCTION__, "HTTP_EVENT_ON_HEADER");
        ESP_LOGD(__FUNCTION__, "%.*s", evt->data_len, (char*)evt->data);
        break;
      case HTTP_EVENT_ON_DATA:
        ESP_LOGI(__FUNCTION__, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        ESP_LOGD(__FUNCTION__, "%.*s", evt->data_len, (char*)evt->data);

        break;
      case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(__FUNCTION__, "HTTP_EVENT_ON_FINISH");
        break;
      case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(__FUNCTION__, "HTTP_EVENT_DISCONNECTED");
        break;
    }
    return ESP_OK;
  }
  esp_http_client_config_t config = {
    .url = CHORDER_POST_TARGET,
    //.use_global_ca_store = true,
    .cert_pem = CHORDER_POST_SERVER_CERT,
    .client_cert_pem = CHORDER_POST_CLIENT_CERT,
    .client_key_pem = CHORDER_POST_CLIENT_KEY,
    .event_handler = _http_event_handle,
  };
  esp_http_client_handle_t client = esp_http_client_init(&config);

  esp_http_client_set_method(client, HTTP_METHOD_POST);
  strcpy(encoded,CHORDER_POST_PARMNAME "=");
  urlencode_into(encoded+strlen(CHORDER_POST_PARMNAME "="),sizeof(encoded)-strlen(CHORDER_POST_PARMNAME "="),note);
  ESP_LOGI(__FUNCTION__,"CHORDER_POST_PARMNAME: %s",CHORDER_POST_PARMNAME);
  ESP_LOGI(__FUNCTION__,"note: %s",note);
  ESP_LOGI(__FUNCTION__,"encoded: %s",encoded);
  esp_http_client_set_post_field(client, encoded, strlen(encoded));
  esp_err_t err = esp_http_client_perform(client);
  /* int flags = mbedtls_ssl_get_verify_result(&tls->ssl); */
  /* char buf[100] = { 0, }; */
  /* mbedtls_x509_crt_verify_info(buf, sizeof(buf), " ! ",  flags); */
  /* printf("Certificate Verification Failure Reason: %s\n", buf); */

  if (err == ESP_OK) {
    ESP_LOGI(__FUNCTION__, "Status = %d, content_length = %d",
        esp_http_client_get_status_code(client),
        esp_http_client_get_content_length(client));
  }
  if (err == ESP_OK && 200 == esp_http_client_get_status_code(client)) {
    esp_http_client_cleanup(client);
    return true;
  } else {
    esp_http_client_cleanup(client);
    return false;
  }
}
void switch_to_opmode(enum Operating_mode target);

void set_up_input_pin (int pinnum)
{
    gpio_reset_pin(pinnum);
    if (ESP_OK != gpio_set_direction(pinnum, GPIO_MODE_INPUT))
    {
        ESP_LOGE(__FUNCTION__,"Failure setting pin %d's direction", pinnum);
        return;
    }
    if (ESP_OK != gpio_pullup_en(pinnum))
    {
        ESP_LOGE(__FUNCTION__,"Failure setting pin %d's to pull-up", pinnum);
        return;
    }
    ESP_LOGI(__FUNCTION__,"Successfully set up GPIO pin %d", pinnum);
}
 
void set_up_gnd_pin (int pinnum)
{
    gpio_reset_pin(pinnum);
    if (ESP_OK != gpio_set_direction(pinnum, GPIO_MODE_OUTPUT))
    {
        ESP_LOGE(__FUNCTION__,"Failure setting pin %d's direction", pinnum);
        return;
    }
    if (ESP_OK != gpio_set_level(pinnum, 0))
    {
        ESP_LOGE(__FUNCTION__,"Failure pin %d to level 0", pinnum);
        return;
    }
    ESP_LOGI(__FUNCTION__,"Successfully set up software GND pin %d", pinnum);
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
  ESP_LOGD("sendRawKey","esp_hidd_send_keyboard_value(%d,0x%x,0x%x %x %x %x %x %x %x %x)",
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

bool opmode_switch_and_deepsleep_handler (uint8_t keyState)
{
  symbol_t symbol = keymap[keyState][0];
  switch (symbol) {
    case MODE_BLE_KEYBOARD:
      switch_to_opmode(OPMODE_BLE_KEYBOARD);
      return true;
    case MODE_BLE_MOUSE:
      switch_to_opmode(OPMODE_BLE_MOUSE);
      return true;
    case MODE_NOTETAKING:
      switch_to_opmode(OPMODE_NOTETAKING);
      return true;
    case MODE_DEEPSLEEP:
      ESP_LOGI(__FUNCTION__,"Entering deep sleep now...");
      vTaskDelay(1000 / portTICK_RATE_MS);
      send_chorder_to_sleep();
      return true; // oughtn't actually matter; however, warnings
    default:
      return false;
  }
}

/* Redirector, so that the core logic (shifted not shifted) can be managed
 * centrally:
 */
void handle_keystate_update_internally(uint8_t keyState, void (*symbol_handler)(symbol_t input))
{
  static bool is_shifted = false;
  static bool is_numsymed = false;

  display_timeout_last_activity = xTaskGetTickCount();
  symbol_t symbol = keymap[keyState][is_numsymed ? 6 : (is_shifted? 4 : 4)];

  switch (symbol) {
    case MOD_LSHIFT:
      is_shifted = !is_shifted;
      is_numsymed = false;
      return;
    case MOD_RSHIFT:
      is_shifted = !is_shifted;
      is_numsymed = false;
      return;
    case MODE_NUM:
      is_shifted = false;
      is_numsymed = true;
      return;
    default:
      (*symbol_handler)(symbol);
      is_shifted = false;
      is_numsymed = false;
  }
}

void printing_handler(symbol_t symbol){
  static bool initialised = false;

  if (!initialised)
  {
    lcd_state.message[0] = '\0';
    lcd_state.alert[0] = '\0';
    initialised = true;
  }
  
  switch (symbol) {
    case NONBLE_BACKSPACE:
      lcd_state.message[0 == strlen(lcd_state.message) ? 0 : strlen(lcd_state.message)-1] = '\0';
      break;
    case '\n': // sending on enter key presses:
      if (send_off_note(lcd_state.message)) {
        strcpy(lcd_state.message,"");
      } else {
        strcpy(lcd_state.alert,"Couldn't send note!");
      }
      break;
    default:
      if (symbol < 128) {
        // Keep replacing the last character if we're at the buffer size:
        size_t pos = ((strlen(lcd_state.message) >= INTERNAL_BUFSIZE - 1) ? strlen(lcd_state.message)-1 : strlen(lcd_state.message));
        lcd_state.message[pos] = (char) symbol;
        lcd_state.message[pos+1] = '\0';
      } else {
        sprintf((char *)lcd_state.alert,"Special: %u",symbol);
      }
      break;
  }

}

void handle_keystate_update_internally_with_printing(uint8_t keyState){
  handle_keystate_update_internally(keyState,&printing_handler);
}

void handle_keystate_update_as_ble_keyboard(uint8_t keyState){

  display_timeout_last_activity = xTaskGetTickCount();

  keymap_t theKey;  
  // Determine the key based on the current mode's keymap
  if (mode == ALPHA) {
    theKey = keymap[keyState][0];
  } else if (mode == NUMSYM) {
    theKey = keymap[keyState][1];
  } else {
    theKey = keymap[keyState][2];
  }

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

void handle_keystate_update_as_ble_mouse(uint8_t keyState){
  static size_t jump = 8;
  keymap_t theKey;  

  display_timeout_last_activity = xTaskGetTickCount();
  theKey = keymap[keyState][3];

  switch (theKey)  {
    case MODE_NOTETAKING:
      jump = 8;
      switch_to_opmode(OPMODE_NOTETAKING);
      return;
    case MODE_BLE_KEYBOARD:
      jump = 8;
      switch_to_opmode(OPMODE_BLE_KEYBOARD);
      return;
    case BLEMOUSE_LEFT:
      if (sec_conn) esp_hidd_send_mouse_value(hid_conn_id,0,-jump,0,0);
      break;
    case BLEMOUSE_DOWN:
      if (sec_conn) esp_hidd_send_mouse_value(hid_conn_id,0,0,-jump,0);
      break;
    case BLEMOUSE_UP:
      if (sec_conn) esp_hidd_send_mouse_value(hid_conn_id,0,0,jump,0);
      break;
    case BLEMOUSE_RIGHT:
      if (sec_conn) esp_hidd_send_mouse_value(hid_conn_id,0,jump,0,0);
      break;
    case BLEMOUSE_1CLICK:
      if (sec_conn) {
        esp_hidd_send_mouse_value(hid_conn_id,1,0,0,0);
        esp_hidd_send_mouse_value(hid_conn_id,0,0,0,0);
      }
      break;
    case BLEMOUSE_FURTHER:
      jump = 127 > 2 * jump ? 2 * jump : 127;
      sprintf(lcd_state.alert,"Jump: %d",jump);
      break;
    case BLEMOUSE_SHORTER:
      jump = 0 < jump / 2 ? jump / 2 : 1;
      sprintf(lcd_state.alert,"Jump: %d",jump);
      break;
    default:
      strcpy(lcd_state.alert,"Unknown\nkey");
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

void switch_to_opmode(enum Operating_mode target){
  switch(target) {
    case OPMODE_NOTETAKING:
      keystate_handler = &handle_keystate_update_internally_with_printing;
      lcd_style.background_color = BLACK;
      break;
    case OPMODE_BLE_KEYBOARD:
      keystate_handler = &handle_keystate_update_as_ble_keyboard;
      mode = ALPHA;
      modKeys = 0x00;
      isCapsLocked = false;
      isNumsymLocked = false;
      lcd_style.background_color = BLUE;
      break;
    case OPMODE_BLE_MOUSE:
      keystate_handler = &handle_keystate_update_as_ble_mouse;
      lcd_style.background_color = CYAN;
      break;
    default:
      ESP_LOGE(__FUNCTION__,"Wrong switch_to_mode chosen.");
  }
}

// }}}

////////////////////////////////////////////////////////////////////////////////
// Main FreeRTOS tasks
////////////////////////////////////////////////////////////////////////////////
// {{{
void watch_for_key_changes (void *pvParameters)
{
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
            ESP_LOGD(__FUNCTION__, "New reading: %s%s%s %s%s%s%s",
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
                    ESP_LOGI(__FUNCTION__, "Performing a BT factory reset: ");
                    if ( ! ble.factoryReset() ){
                        ESP_LOGW(__FUNCTION__, "Factory reset failed!");
                    }
                    ESP_LOGI(__FUNCTION__, "Resetting Arduino...");
                    resetFunc();
                }
            */
            }
            have_seen_first_stable_reading = true;
            switch (state) {
              case PRESSING:
                if (previousStableReading & ~currentStableReading) {
                  state = RELEASING;
                  // First, let the opmode_switch_handler react. If it does nothing, proceed:
                  if (! opmode_switch_and_deepsleep_handler(previousStableReading))
                  {
                    (*keystate_handler)(previousStableReading);
                  }
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

// }}}


void app_main(void)
{
    esp_err_t ret;
    initialize_lcd();
    // Initialize FreeRTOS elements
    eventgroup_system = xEventGroupCreate();
    if(eventgroup_system == NULL) ESP_LOGE(__FUNCTION__, "Cannot initialize event group");
    //if set in KConfig, pairing is disable by default.
    //User has to enable pairing with $PM1
#if CONFIG_MODULE_BT_PAIRING
    ESP_LOGI(__FUNCTION__,"pairing disabled by default");
    xEventGroupClearBits(eventgroup_system,SYSTEM_PAIRING_ENABLED);
    hidd_adv_params.adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_WLST;
#else
    ESP_LOGI(__FUNCTION__,"pairing enabled by default");
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
        ESP_LOGE(__FUNCTION__, "%s initialize controller failed\n", __func__);
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(__FUNCTION__, "%s enable controller failed\n", __func__);
        return;
    }

    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(__FUNCTION__, "%s init bluedroid failed\n", __func__);
        return;
    }

    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(__FUNCTION__, "%s init bluedroid failed\n", __func__);
        return;
    }

    if((ret = esp_hidd_profile_init()) != ESP_OK) {
        ESP_LOGE(__FUNCTION__, "%s init bluedroid failed\n", __func__);
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

    spi_master_init(&dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO, CONFIG_BL_GPIO);

    wifi_init_sta();

    // Chorder setup
    switch_to_opmode(OPMODE_BLE_KEYBOARD);

    // Initialise LCD, set fonts etc



    //xTaskCreate(ST7789, "ST7789", 1024*6, NULL, 2, NULL);
    //xTaskCreate(BLE_muckery, "BLE_muckery", 1024*6, NULL, 2, NULL);
    xTaskCreate(render_display_task, "render_display_task", 1024*3, NULL, 2, NULL);
    xTaskCreate(watch_for_key_changes, "watch_for_key_changes", 1024*6, NULL, 2, NULL);
}
