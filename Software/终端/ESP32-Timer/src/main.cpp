#include "Arduino.h"

hw_timer_t *timer = NULL;
uint32_t flag = 0;

static int pinOut = D6;

static void IRAM_ATTR Timer0_CallBack(void) {
    flag = 1;
}

void setup() {
    Serial.begin(115200);
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &Timer0_CallBack, true);
    timerAlarmWrite(timer, 1000000, true); // This is 
    timerAlarmEnable(timer);
    pinMode(pinOut, OUTPUT);
    digitalWrite(pinOut, LOW);
}

void loop() {
    if (flag) {
        Serial.println("Timer0_CallBack");
        digitalWrite(pinOut, !digitalRead(pinOut));
        flag = 0;
    }
}