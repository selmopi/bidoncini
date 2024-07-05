#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <esp_err.h>
#include <time.h>

#include "lora.h"
#include "../../../components/ra01s/ra01s.h"

char *TAG_LORA = "LORA_MODULE";

void initialize_lora(){
    uint8_t spreadingFactor = 7;
	uint8_t bandwidth = 4;
	uint8_t codingRate = 1;
	uint16_t preambleLength = 8;
	uint8_t payloadLen = 0;
	bool crcOn = true;
	bool invertIrq = false;
	LoRaInit();
	LoRaBegin(LORA_FREQUENCY, 22, 3.3, true);
    LoRaConfig(spreadingFactor, bandwidth, codingRate, preambleLength, payloadLen, crcOn, invertIrq);
}

void lora_message_send(char* ID, int distance){
    uint8_t buf[256]; // Maximum Payload size of SX1261/62/68 is 255

    //get current time
	time_t now;
	char strftime_buf[64];
	struct tm timeinfo;
	time(&now);
	localtime_r(&now, &timeinfo);
	strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
	ESP_LOGI(TAG_LORA, "%s", strftime_buf);

    //build the message
	int txLen = sprintf((char *)buf, "{\"id\": \"%s\", \"distance\": %d}", 
							ID, distance);
		
	ESP_LOGI(TAG_LORA, "%d byte packet sent...", txLen);

	// Wait for transmission to complete
	if (LoRaSend(buf, txLen, SX126x_TXMODE_SYNC) == false) {
		ESP_LOGE(TAG_LORA,"LoRaSend fail");
	}
	ESP_LOGI(TAG_LORA, "LoRa packet sent");

    //check for packets lost
	int lost = GetPacketLost();
	if (lost != 0) {
		ESP_LOGW(TAG_LORA, "%d packets lost", lost);
	}
}