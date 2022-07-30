#include <stdlib.h>
#include "UC8151c_152x152.h"

UC8151c_152x152::~UC8151c_152x152() {
};

UC8151c_152x152::UC8151c_152x152() {
    reset_pin = RST_PIN;
    dc_pin = DC_PIN;
    cs_pin = CS_PIN;
    busy_pin = BUSY_PIN;
};

unsigned char UC8151c_152x152::Init(void) {
    if (IfInit() != 0) {
        return -1;
    }
    
    Reset();
    SendCommand(POWER_SETTING);
    SendData(0x03);
    SendData(0x00);
    SendData(0x2B);
    SendData(0x2B);
    SendData(0x29);
    SendCommand(BOOSTER_SOFT_START);
    SendData(0x17);
    SendData(0x17);
    SendData(0x17);
    
    SendCommand(POWER_ON);

    WaitUntilIdle();

    SendCommand(PANEL_SETTING);
    SendData(0xCF);
    SendCommand(VCOM_AND_DATA_INTERVAL_SETTING);
    SendData(0x17);
    SendCommand(PLL_CONTROL);
    SendData(0x39);
    SendCommand(TCON_RESOLUTION);
    SendData(width);
    SendData(height>>8);
    SendData(height);
    SendCommand(VCM_DC_SETTING_REGISTER);
    SendData(0x0E);

    return 0;
}

/**
 *  @brief: basic function for sending commands
 */
void UC8151c_152x152::SendCommand(unsigned char command) {
    digitalWrite(dc_pin, LOW);
    Transfer(command);
}

/**
 *  @brief: basic function for sending data
 */
void UC8151c_152x152::SendData(unsigned char data) {
    digitalWrite(dc_pin, HIGH);
    Transfer(data);
}

/**
 *  @brief: Wait until the busy_pin goes HIGH
 */
void UC8151c_152x152::WaitUntilIdle(void) {
    while(digitalRead(busy_pin) == LOW) {    //LOW: busy, HIGH: idle
        delay(100);
    }      
}

/**
 *  @brief: module reset.
 *          often used to awaken the module in deep sleep,
 *          see UC8151c_152x152::Sleep();
 */
void UC8151c_152x152::Reset(void) {
    digitalWrite(reset_pin, LOW);                //module reset    
    delay(200);
    digitalWrite(reset_pin, HIGH);
    delay(200);    
}

/**
 *  @brief: set the Black-White look-up tables
 */
void UC8151c_152x152::SetLutBw(void) {
}

/**
 *  @brief: set the Red look-up tables
 */
void UC8151c_152x152::SetLutRed(void) {
}

/**
 *  @brief: put an image buffer to the Pattern memory.
 */
void UC8151c_152x152::DisplayPattern(const unsigned char* pattern_buffer_black, const unsigned char* pattern_buffer_red) {
    if (pattern_buffer_black != NULL) {
        SendCommand(DATA_START_TRANSMISSION_1);
        delay(2);
        for (unsigned int i = 0; i < this->width * this->height / 8; i++) {
            SendData(pgm_read_byte(&pattern_buffer_black[i]));
        }
        delay(2);
    }
    if (pattern_buffer_red != NULL) {
        SendCommand(DATA_START_TRANSMISSION_2);
        delay(2);
        for (unsigned int i = 0; i < this->width * this->height / 8; i++) {
            SendData(pgm_read_byte(&pattern_buffer_red[i]));
        }
        delay(2);
    }
    SendCommand(DISPLAY_REFRESH);
    WaitUntilIdle();
}

/**
 *  @brief: After this command is transmitted, the chip would enter the 
 *          deep-sleep mode to save power. 
 *          The deep sleep mode would return to standby by hardware reset. 
 *          The only one parameter is a check code, the command would be
 *          executed if check code = 0xA5. 
 *          You can use UC8151c_152x152::Init() to awaken
 */
void UC8151c_152x152::Sleep() {
    SendCommand(VCOM_AND_DATA_INTERVAL_SETTING);
    SendData(0x17);
    SendCommand(VCM_DC_SETTING_REGISTER);         //to solve Vcom drop
    SendData(0x00);
    SendCommand(POWER_SETTING);         //power setting
    SendData(0x02);        //gate switch to external
    SendData(0x00);
    SendData(0x00);
    SendData(0x00);
    WaitUntilIdle();
    SendCommand(POWER_OFF);         //power off
}

const unsigned char UC8151c_152x152::lut_vcom0[] = {};

const unsigned char UC8151c_152x152::lut_w[] = {};

const unsigned char UC8151c_152x152::lut_b[] = {};

const unsigned char UC8151c_152x152::lut_g1[] = {};

const unsigned char UC8151c_152x152::lut_g2[] = {};

const unsigned char UC8151c_152x152::lut_vcom1[] = {};

const unsigned char UC8151c_152x152::lut_red0[] = {};

const unsigned char UC8151c_152x152::lut_red1[] = {};
