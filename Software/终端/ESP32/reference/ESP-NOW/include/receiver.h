
#ifndef __receiver_h__
#define __receiver_h__
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include "data.h"

// Create a struct_message called myData
extern struct_message recData;

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len);
void setup_receiver();

#endif  // __receiver_h__