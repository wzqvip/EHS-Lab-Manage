// Complete Instructions to Get and Change ESP MAC Address: https://RandomNerdTutorials.com/get-change-esp32-esp8266-mac-address-arduino/

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

#include "receiver.h"
#include "sender.h"

const int LED_1 = D6;
const int LED_2 = D7;

#define TEST 0
#define MASTER 1
#define SLAVE 1

#if TEST
void setup() {
    Serial.begin(115200);
    WiFi.mode(WIFI_MODE_STA);
    delay(1000);
    Serial.println(WiFi.macAddress());
}

void loop() {
    Serial.println(WiFi.macAddress());
    delay(1000);
}

#elif MASTER

void setup() {
    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
    setup_sender();
}

void loop() {
    sender();
}

#elif SLAVE

void setup() {
    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
    setup_receiver();
}

void loop() {
    // put your main code here, to run repeatedly:
}

#endif