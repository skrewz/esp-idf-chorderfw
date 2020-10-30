
#ifndef _CONFIG_H_
#define _CONFIG_H_

#define MODULE_ID "ESP32miniBT_v0.1"

#if CONFIG_MODULE_FLIPMOUSE
    #define GATTS_TAG "FLipMouse"
#else
    #if CONFIG_MODULE_FABI
        #define GATTS_TAG "FABI"
    #else
        #define GATTS_TAG "esp32_mouse_keyboard"
    #endif
#endif

#define MAX_BT_DEVICENAME_LENGTH 40

// serial port of monitor and for debugging (not in KConfig, won't be changed normally)
#define CONSOLE_UART_NUM 	 UART_NUM_0

// serial port for connection to other controllers
#define EX_UART_NUM     	 CONFIG_MODULE_UART_NR
#define EX_SERIAL_TXPIN      CONFIG_MODULE_TX_PIN
#define EX_SERIAL_RXPIN      CONFIG_MODULE_RX_PIN

// indicator LED
#define INDICATOR_LED_PIN    CONFIG_MODULE_LED_PIN

typedef struct config_data {
    char bt_device_name[MAX_BT_DEVICENAME_LENGTH];
    uint8_t locale;
} config_data_t;

#define INTERNAL_BUFSIZE 200

#define TAG "ST7789"

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

#endif
