#ifndef __UC8151c_152x152_H__
#define __UC8151c_152x152_H__

#include <epdif.h>

// EPD 1.54 Inch RED WHITE BLACK commands
#define PANEL_SETTING                               0x00
#define POWER_SETTING                               0x01
#define POWER_OFF                                   0x02
#define POWER_OFF_SEQUENCE_SETTING                  0x03
#define POWER_ON                                    0x04
#define POWER_ON_MEASURE                            0x05
#define BOOSTER_SOFT_START                          0x06
#define DEEP_SLEEP                                  0x07
#define DATA_START_TRANSMISSION_1                   0x10
#define DATA_STOP                                   0x11
#define DISPLAY_REFRESH                             0x12
#define DATA_START_TRANSMISSION_2                   0x13
#define PLL_CONTROL                                 0x30
#define TEMPERATURE_SENSOR_COMMAND                  0x40
#define TEMPERATURE_SENSOR_CALIBRATION              0x41
#define TEMPERATURE_SENSOR_WRITE                    0x42
#define TEMPERATURE_SENSOR_READ                     0x43
#define VCOM_AND_DATA_INTERVAL_SETTING              0x50
#define LOW_POWER_DETECTION                         0x51
#define TCON_SETTING                                0x60
#define TCON_RESOLUTION                             0x61
#define SOURCE_AND_GATE_START_SETTING               0x62
#define GET_STATUS                                  0x71
#define AUTO_MEASURE_VCOM                           0x80
#define VCOM_VALUE                                  0x81
#define VCM_DC_SETTING_REGISTER                     0x82
#define PROGRAM_MODE                                0xA0
#define ACTIVE_PROGRAM                              0xA1
#define READ_OTP_DATA                               0xA2

#define Lut_length 15

class UC8151c_152x152 : EpdIf {
public:
    unsigned long width = 152;
    unsigned long height = 152;

    UC8151c_152x152();
    ~UC8151c_152x152();
    unsigned char Init(void);
    void SendCommand(unsigned char command);
    void SendData(unsigned char data);
    void WaitUntilIdle(void);
    void Reset(void);
    void DisplayPattern(const unsigned char* pattern_buffer_black, const unsigned char* pattern_buffer_red);
    void Sleep(void);

private:
    unsigned int reset_pin;
    unsigned int dc_pin;
    unsigned int cs_pin;
    unsigned int busy_pin;

    static const unsigned char lut_vcom0[Lut_length];
    static const unsigned char lut_vcom1[Lut_length];
    static const unsigned char lut_w[Lut_length];
    static const unsigned char lut_b[Lut_length];
    static const unsigned char lut_g1[Lut_length];
    static const unsigned char lut_g2[Lut_length];
    static const unsigned char lut_red0[Lut_length];
    static const unsigned char lut_red1[Lut_length];
    
    void SetLutBw(void);
    void SetLutRed(void);
};

#endif
