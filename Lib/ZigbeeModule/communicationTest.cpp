#include <Arduino.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiAvrI2c.h>
#include <SoftwareSerial.h>

#define I2C_ADDRESS 0x3C

#define RST_PIN -1

SSD1306AsciiAvrI2c oled;
SoftwareSerial ZigbeeSerial(2, 3);
//------------------------------------------------------------------------------
void oledDisplay(uint8_t x, uint8_t y, String s) {
    oled.println("Received Message!");
    oled.set2X();
    oled.println(s);
    oled.set1X();
    oled.println("Displaying Message!");
}

void setup() {
#if RST_PIN >= 0
    oled.begin(&Adafruit128x32, I2C_ADDRESS, RST_PIN);
#else   // RST_PIN >= 0
    oled.begin(&Adafruit128x32, I2C_ADDRESS);
#endif  // RST_PIN >= 0
    oled.setI2cClock(20000);
    oled.setFont(Adafruit5x7);
    oled.clear();

    Serial.begin(115200);
    while (!Serial) {}
    Serial.println("Serial Started");
    ZigbeeSerial.begin(115200);
    ZigbeeSerial.println("ZigbeeSerial Started");
}
//------------------------------------------------------------------------------
void loop() {
  String msg;
  int count = 0;
  if (ZigbeeSerial.available()) {
    char c = ZigbeeSerial.read();
    while (c != '!'&& count<100 ) {
      msg += c;
      count++;
      Serial.print(c);
      c = ZigbeeSerial.read();
    }
  }
  
  if (msg.length() > 0) {
    oled.clear();
    oledDisplay(0, 0, msg);
  }
}