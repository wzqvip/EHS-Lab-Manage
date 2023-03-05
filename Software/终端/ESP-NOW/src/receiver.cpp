#include "receiver.h"

struct_message recData;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
    digitalWrite(LED_1, !digitalRead(LED_1));
    memcpy(&recData, incomingData, sizeof(recData));
    Serial.print("Bytes received: ");
    Serial.println(len);
    Serial.print("Char: ");
    Serial.println(recData.a);
    Serial.print("Int: ");
    Serial.println(recData.b);
    Serial.print("Float: ");
    Serial.println(recData.c);
    Serial.print("Bool: ");
    Serial.println(recData.d);
    Serial.println();
}

void setup_receiver() {
    // Initialize Serial Monitor
    Serial.begin(115200);

    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        digitalWrite(LED_2, HIGH);
        return;
    }

    // Once ESPNow is successfully Init, we will register for recv CB to
    // get recv packer info
    esp_now_register_recv_cb(OnDataRecv);
}
