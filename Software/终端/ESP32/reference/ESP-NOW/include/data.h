#ifndef __data_h__
#define __data_h__

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
    char a[32];
    int b;
    float c;
    bool d;
} struct_message;

extern const int LED_1;
extern const int LED_2;

#endif  // __data_h__