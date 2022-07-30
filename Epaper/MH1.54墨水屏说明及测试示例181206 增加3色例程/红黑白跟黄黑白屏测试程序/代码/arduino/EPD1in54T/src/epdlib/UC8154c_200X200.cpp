#include <stdlib.h>
#include "UC8154c_200x200.h"

UC8154c_200x200::~UC8154c_200x200() {
};

UC8154c_200x200::UC8154c_200x200() {
    reset_pin = RST_PIN;
    dc_pin = DC_PIN;
    cs_pin = CS_PIN;
    busy_pin = BUSY_PIN;
};

unsigned char UC8154c_200x200::Init(void) {
    if (IfInit() != 0) {
        return -1;
    }
    
    Reset();
    SendCommand(POWER_SETTING);
    SendData(0x07);
    SendData(0x00);
    SendData(0x08);
    SendData(0x00);

    SendCommand(BOOSTER_SOFT_START);
    SendData(0x07);
    SendData(0x07);
    SendData(0x07);
    
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

    SetLutBw();
    SetLutRed();

    return 0;
}

/**
 *  @brief: basic function for sending commands
 */
void UC8154c_200x200::SendCommand(unsigned char command) {
    digitalWrite(dc_pin, LOW);
    Transfer(command);
}

/**
 *  @brief: basic function for sending data
 */
void UC8154c_200x200::SendData(unsigned char data) {
    digitalWrite(dc_pin, HIGH);
    Transfer(data);
}

/**
 *  @brief: Wait until the busy_pin goes HIGH
 */
void UC8154c_200x200::WaitUntilIdle(void) {
    while(digitalRead(busy_pin) == LOW) {    //LOW: busy, HIGH: idle
        delay(100);
    }      
}

/**
 *  @brief: module reset.
 *          often used to awaken the module in deep sleep,
 *          see UC8154c_200x200::Sleep();
 */
void UC8154c_200x200::Reset(void) {
    digitalWrite(reset_pin, LOW);                //module reset    
    delay(200);
    digitalWrite(reset_pin, HIGH);
    delay(200);    
}

/**
 *  @brief: set the Black-White look-up tables
 */
void UC8154c_200x200::SetLutBw(void) {

    unsigned int count;

    SendCommand(0x20);         //g vcom
    for(count = 0; count < 15; count++) {
        SendData(UC8154c_200x200::lut_vcom0[count]);
    }
    SendCommand(0x21);         //g ww --
    for(count = 0; count < 15; count++) {
        SendData(UC8154c_200x200::lut_w[count]);
    }
    SendCommand(0x22);         //g bw r
    for(count = 0; count < 15; count++) {
        SendData(UC8154c_200x200::lut_b[count]);
    }
    SendCommand(0x23);         //g wb w
    for(count = 0; count < 15; count++) {
        SendData(UC8154c_200x200::lut_g1[count]);
    }
    SendCommand(0x24);         //g bb b
    for(count = 0; count < 15; count++) {
        SendData(UC8154c_200x200::lut_g2[count]);
    }
}

/**
 *  @brief: set the Red look-up tables
 */
void UC8154c_200x200::SetLutRed(void) {

    unsigned int count;

    SendCommand(0x25);
    for(count = 0; count < 15; count++) {
        SendData(UC8154c_200x200::lut_vcom1[count]);
    } 
    SendCommand(0x26);
    for(count = 0; count < 15; count++) {
        SendData(UC8154c_200x200::lut_red0[count]);
    } 
    SendCommand(0x27);
    for(count = 0; count < 15; count++) {
        SendData(UC8154c_200x200::lut_red1[count]);
    }
}

/**
 *  @brief: put an image buffer to the Pattern memory.
 */
void UC8154c_200x200::DisplayPattern(const unsigned char* pattern_buffer_black, const unsigned char* pattern_buffer_red) {

    unsigned char temp;
    if (pattern_buffer_black != NULL) {
        SendCommand(DATA_START_TRANSMISSION_1);
        delay(2);
        for (unsigned int i = 0; i < this->width * this->height / 8; i++) {
            temp = 0x00;
            for (int bit = 0; bit < 4; bit++) {
                if ((pgm_read_byte(&pattern_buffer_black[i]) & (0x80 >> bit)) != 0) {
                    temp |= 0xC0 >> (bit * 2);
                }
            }
            SendData(temp);
            temp = 0x00;
            for (int bit = 4; bit < 8; bit++) {
                if ((pgm_read_byte(&pattern_buffer_black[i]) & (0x80 >> bit)) != 0) {
                    temp |= 0xC0 >> ((bit - 4) * 2);
                }
            }
            SendData(temp);
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
 *          You can use UC8154c_200x200::Init() to awaken
 */
void UC8154c_200x200::Sleep() {
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

const unsigned char UC8154c_200x200::lut_vcom0[] =
{
    0x0E, 0x14, 0x01, 0x0A, 0x06, 0x04, 0x0A, 0x0A,
    0x0F, 0x03, 0x03, 0x0C, 0x06, 0x0A, 0x00
};

const unsigned char UC8154c_200x200::lut_w[] =
{
    0x0E, 0x14, 0x01, 0x0A, 0x46, 0x04, 0x8A, 0x4A,
    0x0F, 0x83, 0x43, 0x0C, 0x86, 0x0A, 0x04
};

const unsigned char UC8154c_200x200::lut_b[] = 
{
    0x0E, 0x14, 0x01, 0x8A, 0x06, 0x04, 0x8A, 0x4A,
    0x0F, 0x83, 0x43, 0x0C, 0x06, 0x4A, 0x04
};

const unsigned char UC8154c_200x200::lut_g1[] = 
{
    0x8E, 0x94, 0x01, 0x8A, 0x06, 0x04, 0x8A, 0x4A,
    0x0F, 0x83, 0x43, 0x0C, 0x06, 0x0A, 0x04
};

const unsigned char UC8154c_200x200::lut_g2[] = 
{
    0x8E, 0x94, 0x01, 0x8A, 0x06, 0x04, 0x8A, 0x4A,
    0x0F, 0x83, 0x43, 0x0C, 0x06, 0x0A, 0x04
};

const unsigned char UC8154c_200x200::lut_vcom1[] = 
{
    0x03, 0x1D, 0x01, 0x01, 0x08, 0x23, 0x37, 0x37,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char UC8154c_200x200::lut_red0[] = 
{
    0x83, 0x5D, 0x01, 0x81, 0x48, 0x23, 0x77, 0x77,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char UC8154c_200x200::lut_red1[] = 
{
    0x03, 0x1D, 0x01, 0x01, 0x08, 0x23, 0x37, 0x37,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};