#include "sender.h"

// 34:85:18:05:A4:90
uint8_t broadcastAddress[] = {0x34, 0x85, 0x18, 0x05, 0xA4, 0x90};

struct_message sendData;
esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup_sender() {
    // Init Serial Monitor
    Serial.begin(115200);

    // Set device as a Wi-Fi Station
    WiFi.mode(WIFI_STA);

    // Init ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
    }

    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);

    // Register peer
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    // Add peer
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Failed to add peer");
        return;
    }

    digitalWrite(LED_1, HIGH);
}

void sender() {
    // Set values to send
    strcpy(sendData.a, "THIS IS A CHAR");
    sendData.b = random(1, 20);
    sendData.c = 1.2;
    sendData.d = false;

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&sendData, sizeof(sendData));
    digitalWrite(LED_1, !digitalRead(LED_1));
    if (result == ESP_OK) {
        Serial.println("Sent with success");
        digitalWrite(LED_2, LOW);
    } else {
        Serial.println("Error sending the data");
        digitalWrite(LED_2, HIGH);
    }
    delay(2000);
}