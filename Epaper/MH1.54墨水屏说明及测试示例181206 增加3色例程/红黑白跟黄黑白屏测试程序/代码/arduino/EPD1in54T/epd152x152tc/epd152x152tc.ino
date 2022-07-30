/*demo for MH-ET LVIE 1.54 Yellow-Black Epaper*/
/*
*connection:
Board.UNO/2560.etc-------------epaper module
D7--------------------------------Busy
D8--------------------------------Reset
D9--------------------------------D/C
D10-------------------------------CS
D11-------------------------------SDI
D13-------------------------------SCLK
VCC-------------------------------VCC
GND------------------------------GND
*Set the toggle switch to the correct gear position according to the supply voltage.
*SPI:4line-wire
*/
#include <EPD_Display.h>
#include "imagedata.h"


void setup() {
    Serial.begin(9600);
    UC8151c_152x152 epd;
    if (epd.Init() != 0) {
        Serial.print("e-Paper init failed");
        return;
    }
    epd.DisplayPattern(IMAGE_BLACK, IMAGE_YELLOW);
}

void loop() {
  // put your main code here, to run repeatedly:

}
