
#ifndef __sender_h__
#define __sender_h__
#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include "data.h"

extern uint8_t broadcastAddress[];
// Create a struct_message called myData
extern struct_message sendData;
extern esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void setup_sender();
void sender();
#endif  // __sender_h__
