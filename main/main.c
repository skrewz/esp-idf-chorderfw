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

#include "hid_dev.h"
#include "config.h"

#include "driver/gpio.h"
#include "st7789.h"
#include "fontx.h"
#include "bmpfile.h"
#include "decode_image.h"
#include "pngle.h"

#define	INTERVAL		400
#define WAIT	vTaskDelay(INTERVAL)

#define TEST_PIN GPIO_NUM_2
#define TEST_GND_PIN GPIO_NUM_15

static const char *TAG = "ST7789";


////////////////////////////////////////////////////////////////////////////////
// Configuration relevant to bluetooth functionality
////////////////////////////////////////////////////////////////////////////////

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
static uint8_t manufacturer[19]= {'A', 's', 'T', 'e', 'R', 'I', 'C', 'S', ' ', 'F', 'o', 'u', 'n', 'd', 'a', 't', 'i', 'o', 'n'};

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

////////////////////////////////////////////////////////////////////////////////
// Configuration relevant to ST7789:
////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////
// Helper functions for bluetooth behaviour
////////////////////////////////////////////////////////////////////////////////


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

TickType_t ColorTest(TFT_t * dev, int width, int height) {
	TickType_t startTick, endTick, diffTick;
	startTick = xTaskGetTickCount();

	uint16_t color;
	lcdFillScreen(dev, WHITE);
	color = RED;
	uint16_t delta = height/16;
	uint16_t ypos = 0;
	for(int i=0;i<16;i++) {
		//ESP_LOGI(__FUNCTION__, "color=0x%x",color);
		lcdDrawFillRect(dev, 0, ypos, width-1, ypos+delta, color);
		color = color >> 1;
		ypos = ypos + delta;
	}

	endTick = xTaskGetTickCount();
	diffTick = endTick - startTick;
	ESP_LOGI(__FUNCTION__, "elapsed time[ms]:%d",diffTick*portTICK_RATE_MS);
	return diffTick;
}


TickType_t BMPTest(TFT_t * dev, char * file, int width, int height) {
	TickType_t startTick, endTick, diffTick;
	startTick = xTaskGetTickCount();

	lcdSetFontDirection(dev, 0);
	lcdFillScreen(dev, BLACK);

	// open requested file
	esp_err_t ret;
	FILE* fp = fopen(file, "rb");
	if (fp == NULL) {
		ESP_LOGW(__FUNCTION__, "File not found [%s]", file);
		return 0;
	}

	// read bmp header
	bmpfile_t *result = (bmpfile_t*)malloc(sizeof(bmpfile_t));
	ret = fread(result->header.magic, 1, 2, fp);
	assert(ret == 2);
	if (result->header.magic[0]!='B' || result->header.magic[1] != 'M') {
		ESP_LOGW(__FUNCTION__, "File is not BMP");
		free(result);
		fclose(fp);
		return 0;
	}
	ret = fread(&result->header.filesz, 4, 1 , fp);
	assert(ret == 1);
	ESP_LOGD(__FUNCTION__,"result->header.filesz=%d", result->header.filesz);
	ret = fread(&result->header.creator1, 2, 1, fp);
	assert(ret == 1);
	ret = fread(&result->header.creator2, 2, 1, fp);
	assert(ret == 1);
	ret = fread(&result->header.offset, 4, 1, fp);
	assert(ret == 1);

	// read dib header
	ret = fread(&result->dib.header_sz, 4, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.width, 4, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.height, 4, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.nplanes, 2, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.depth, 2, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.compress_type, 4, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.bmp_bytesz, 4, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.hres, 4, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.vres, 4, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.ncolors, 4, 1, fp);
	assert(ret == 1);
	ret = fread(&result->dib.nimpcolors, 4, 1, fp);
	assert(ret == 1);

	if((result->dib.depth == 24) && (result->dib.compress_type == 0)) {
		// BMP rows are padded (if needed) to 4-byte boundary
		uint32_t rowSize = (result->dib.width * 3 + 3) & ~3;
		int w = result->dib.width;
		int h = result->dib.height;
		ESP_LOGD(__FUNCTION__,"w=%d h=%d", w, h);
		int _x;
		int _w;
		int _cols;
		int _cole;
		if (width >= w) {
			_x = (width - w) / 2;
			_w = w;
			_cols = 0;
			_cole = w - 1;
		} else {
			_x = 0;
			_w = width;
			_cols = (w - width) / 2;
			_cole = _cols + width - 1;
		}
		ESP_LOGD(__FUNCTION__,"_x=%d _w=%d _cols=%d _cole=%d",_x, _w, _cols, _cole);

		int _y;
		int _rows;
		int _rowe;
		if (height >= h) {
			_y = (height - h) / 2;
			_rows = 0;
			_rowe = h -1;
		} else {
			_y = 0;
			_rows = (h - height) / 2;
			_rowe = _rows + height - 1;
		}
		ESP_LOGD(__FUNCTION__,"_y=%d _rows=%d _rowe=%d", _y, _rows, _rowe);

#define BUFFPIXEL 20
		uint8_t sdbuffer[3*BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
		uint16_t *colors = (uint16_t*)malloc(sizeof(uint16_t) * w);

		for (int row=0; row<h; row++) { // For each scanline...
			if (row < _rows || row > _rowe) continue;
			// Seek to start of scan line.	It might seem labor-
			// intensive to be doing this on every line, but this
			// method covers a lot of gritty details like cropping
			// and scanline padding.  Also, the seek only takes
			// place if the file position actually needs to change
			// (avoids a lot of cluster math in SD library).
			// Bitmap is stored bottom-to-top order (normal BMP)
			int pos = result->header.offset + (h - 1 - row) * rowSize;
			fseek(fp, pos, SEEK_SET);
			int buffidx = sizeof(sdbuffer); // Force buffer reload

			int index = 0;
			for (int col=0; col<w; col++) { // For each pixel...
				if (buffidx >= sizeof(sdbuffer)) { // Indeed
					fread(sdbuffer, sizeof(sdbuffer), 1, fp);
					buffidx = 0; // Set index to beginning
				}
				if (col < _cols || col > _cole) continue;
				// Convert pixel from BMP to TFT format, push to display
				uint8_t b = sdbuffer[buffidx++];
				uint8_t g = sdbuffer[buffidx++];
				uint8_t r = sdbuffer[buffidx++];
				colors[index++] = rgb565_conv(r, g, b);
			} // end for col
			ESP_LOGD(__FUNCTION__,"lcdDrawMultiPixels _x=%d _y=%d row=%d",_x, _y, row);
			//lcdDrawMultiPixels(dev, _x, row+_y, _w, colors);
			lcdDrawMultiPixels(dev, _x, _y, _w, colors);
			_y++;
		} // end for row
		free(colors);
	} // end if
	free(result);
	fclose(fp);

	endTick = xTaskGetTickCount();
	diffTick = endTick - startTick;
	ESP_LOGI(__FUNCTION__, "elapsed time[ms]:%d",diffTick*portTICK_RATE_MS);
	return diffTick;
}


TickType_t JPEGTest(TFT_t * dev, char * file, int width, int height) {
	TickType_t startTick, endTick, diffTick;
	startTick = xTaskGetTickCount();

	lcdSetFontDirection(dev, 0);
	lcdFillScreen(dev, BLACK);


	pixel_s **pixels;
	uint16_t imageWidth;
	uint16_t imageHeight;
	esp_err_t err = decode_image(&pixels, file, width, height, &imageWidth, &imageHeight);
	ESP_LOGI(__FUNCTION__, "decode_image err=%d imageWidth=%d imageHeight=%d", err, imageWidth, imageHeight);
	if (err == ESP_OK) {

		uint16_t _width = width;
		uint16_t _cols = 0;
		if (width > imageWidth) {
			_width = imageWidth;
			_cols = (width - imageWidth) / 2;
		}
		ESP_LOGD(__FUNCTION__, "_width=%d _cols=%d", _width, _cols);

		uint16_t _height = height;
		uint16_t _rows = 0;
		if (height > imageHeight) {
			_height = imageHeight;
			_rows = (height - imageHeight) / 2;
		}
		ESP_LOGD(__FUNCTION__, "_height=%d _rows=%d", _height, _rows);
		uint16_t *colors = (uint16_t*)malloc(sizeof(uint16_t) * _width);

#if 0
		for(int y = 0; y < _height; y++){
			for(int x = 0;x < _width; x++){
				pixel_s pixel = pixels[y][x];
				uint16_t color = rgb565_conv(pixel.red, pixel.green, pixel.blue);
				lcdDrawPixel(dev, x+_cols, y+_rows, color);
			}
			vTaskDelay(1);
		}
#endif

		for(int y = 0; y < _height; y++){
			for(int x = 0;x < _width; x++){
				pixel_s pixel = pixels[y][x];
				colors[x] = rgb565_conv(pixel.red, pixel.green, pixel.blue);
			}
			lcdDrawMultiPixels(dev, _cols, y+_rows, _width, colors);
			vTaskDelay(1);
		}

		free(colors);
		release_image(&pixels, width, height);
		ESP_LOGD(__FUNCTION__, "Finish");
	}

	endTick = xTaskGetTickCount();
	diffTick = endTick - startTick;
	ESP_LOGI(__FUNCTION__, "elapsed time[ms]:%d",diffTick*portTICK_RATE_MS);
	return diffTick;
}

void png_init(pngle_t *pngle, uint32_t w, uint32_t h)
{
		ESP_LOGD(__FUNCTION__, "png_init w=%d h=%d", w, h);
		ESP_LOGD(__FUNCTION__, "screenWidth=%d screenHeight=%d", pngle->screenWidth, pngle->screenHeight);
		pngle->imageWidth = w;
		pngle->imageHeight = h;
		pngle->reduction = false;
		pngle->scale_factor = 1.0;

		// Calculate Reduction
		if (pngle->screenWidth < pngle->imageWidth || pngle->screenHeight < pngle->imageHeight) {
				pngle->reduction = true;
				double factorWidth = (double)pngle->screenWidth / (double)pngle->imageWidth;
				double factorHeight = (double)pngle->screenHeight / (double)pngle->imageHeight;
				pngle->scale_factor = factorWidth;
				if (factorHeight < factorWidth) pngle->scale_factor = factorHeight;
				pngle->imageWidth = pngle->imageWidth * pngle->scale_factor;
				pngle->imageHeight = pngle->imageHeight * pngle->scale_factor;
		}
		ESP_LOGD(__FUNCTION__, "reduction=%d scale_factor=%f", pngle->reduction, pngle->scale_factor);
		ESP_LOGD(__FUNCTION__, "imageWidth=%d imageHeight=%d", pngle->imageWidth, pngle->imageHeight);

}

void png_draw(pngle_t *pngle, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint8_t rgba[4])
{
		ESP_LOGD(__FUNCTION__, "png_draw x=%d y=%d w=%d h=%d", x,y,w,h);
#if 0
		uint8_t r = rgba[0];
		uint8_t g = rgba[1];
		uint8_t b = rgba[2];
#endif

		// image reduction
		uint32_t _x = x;
		uint32_t _y = y;
		if (pngle->reduction) {
				_x = x * pngle->scale_factor;
				_y = y * pngle->scale_factor;
		}
		if (_y < pngle->screenHeight && _x < pngle->screenWidth) {
				pngle->pixels[_y][_x].red = rgba[0];
				pngle->pixels[_y][_x].green = rgba[1];
				pngle->pixels[_y][_x].blue = rgba[2];
		}

}

void png_finish(pngle_t *pngle) {
		ESP_LOGD(__FUNCTION__, "png_finish");
}

TickType_t PNGTest(TFT_t * dev, char * file, int width, int height) {
		TickType_t startTick, endTick, diffTick;
		startTick = xTaskGetTickCount();

		lcdSetFontDirection(dev, 0);
		lcdFillScreen(dev, BLACK);

		// open PNG file
		FILE* fp = fopen(file, "rb");
		if (fp == NULL) {
				ESP_LOGW(__FUNCTION__, "File not found [%s]", file);
				return 0;
		}

		char buf[1024];
		size_t remain = 0;
		int len;

		pngle_t *pngle = pngle_new(width, height);

		pngle_set_init_callback(pngle, png_init);
		pngle_set_draw_callback(pngle, png_draw);
		pngle_set_done_callback(pngle, png_finish);

		double display_gamma = 2.2;
		pngle_set_display_gamma(pngle, display_gamma);


		while (!feof(fp)) {
				if (remain >= sizeof(buf)) {
						ESP_LOGE(__FUNCTION__, "Buffer exceeded");
						while(1) vTaskDelay(1);
				}

				len = fread(buf + remain, 1, sizeof(buf) - remain, fp);
				if (len <= 0) {
						//printf("EOF\n");
						break;
				}

				int fed = pngle_feed(pngle, buf, remain + len);
				if (fed < 0) {
						ESP_LOGE(__FUNCTION__, "ERROR; %s", pngle_error(pngle));
						while(1) vTaskDelay(1);
				}

				remain = remain + len - fed;
				if (remain > 0) memmove(buf, buf + fed, remain);
		}

		fclose(fp);

		uint16_t _width = width;
		uint16_t _cols = 0;
		if (width > pngle->imageWidth) {
				_width = pngle->imageWidth;
				_cols = (width - pngle->imageWidth) / 2;
		}
		ESP_LOGD(__FUNCTION__, "_width=%d _cols=%d", _width, _cols);

		uint16_t _height = height;
		uint16_t _rows = 0;
		if (height > pngle->imageHeight) {
				_height = pngle->imageHeight;
				_rows = (height - pngle->imageHeight) / 2;
		}
		ESP_LOGD(__FUNCTION__, "_height=%d _rows=%d", _height, _rows);
		uint16_t *colors = (uint16_t*)malloc(sizeof(uint16_t) * _width);

#if 0
		for(int y = 0; y < _height; y++){
				for(int x = 0;x < _width; x++){
						pixel_png pixel = pngle->pixels[y][x];
						uint16_t color = rgb565_conv(pixel.red, pixel.green, pixel.blue);
						lcdDrawPixel(dev, x+_cols, y+_rows, color);
				}
		}
#endif

		for(int y = 0; y < _height; y++){
				for(int x = 0;x < _width; x++){
						pixel_png pixel = pngle->pixels[y][x];
						colors[x] = rgb565_conv(pixel.red, pixel.green, pixel.blue);
						//uint16_t color = rgb565_conv(pixel.red, pixel.green, pixel.blue);
						//colors[x] = ~color;
				}
				lcdDrawMultiPixels(dev, _cols, y+_rows, _width, colors);
				vTaskDelay(1);
		}
		free(colors);
		pngle_destroy(pngle, width, height);

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

                // random squares
		FillRectTest(&dev, CONFIG_WIDTH, CONFIG_HEIGHT);
		WAIT;

                // rainbows
		ColorTest(&dev, CONFIG_WIDTH, CONFIG_HEIGHT);
		WAIT;

		lcdDisplayOff(&dev);
		lcdBacklightOff(&dev);
		WAIT;
		lcdDisplayOn(&dev);
		lcdBacklightOn(&dev);

                // moar rainbows
		ColorTest(&dev, CONFIG_WIDTH, CONFIG_HEIGHT);
		WAIT;
		char file[32];

		strcpy(file, "/spiffs/esp32.jpeg");
		JPEGTest(&dev, file, CONFIG_WIDTH, CONFIG_HEIGHT);
		WAIT;

	} // end while

	// never reach
	while (1) {
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}

void Test_pin_interaction (void *pvParameters)
{
  gpio_reset_pin(TEST_PIN);
  gpio_set_direction(TEST_PIN, GPIO_MODE_INPUT);

  gpio_reset_pin(TEST_GND_PIN);
  gpio_set_direction(TEST_GND_PIN, GPIO_MODE_OUTPUT);
  gpio_set_level(TEST_GND_PIN, 0);

  uint8_t kbdcmd[] = {28};
  int last_read = 0;
  int just_read = 0;
  while(1) {
    just_read = gpio_get_level(TEST_PIN);
    if (1 == just_read && just_read != last_read) {
      kbdcmd[0] = 28;
      esp_hidd_send_keyboard_value(hid_conn_id,0,kbdcmd,1);
      kbdcmd[0] = 0;
      esp_hidd_send_keyboard_value(hid_conn_id,0,kbdcmd,1);
    }
    last_read = just_read;
    vTaskDelay(100 / portTICK_RATE_MS);
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
	xTaskCreate(BLE_muckery, "BLE_muckery", 1024*6, NULL, 2, NULL);
	xTaskCreate(Test_pin_interaction, "Test_pin_interaction", 1024*3, NULL, 2, NULL);
}
