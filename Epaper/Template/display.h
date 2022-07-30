#ifndef __CC2530_DISPLAY_H__
#define __CC2530_DISPLAY_H__

#include "types.h"


#define SPI_CLK     P1_2    //  CLK
#define SPI_DIN     P1_3    //  DIN
#define SPI_DC      P1_4    //  DC
#define SPI_CS      P1_5    //  CS
#define SPI_BUSY    P1_6    //  Busy
#define SPI_RST     P1_7    //  RST

#define EPD_2IN9B_V3_WIDTH       128
#define EPD_2IN9B_V3_HEIGHT      296

void SPI_Delay_ms(unsigned int ms) {
    unsigned int a;
    while(ms) {
        a = 1800;
        while(a--);
        ms--;
    }
    return;
}

void SPI_SendData(unsigned char dat) {
    unsigned char bit = 8, tmp = 0;
    SPI_DC = 1;
    SPI_CS = 0;

    for (bit = 0; bit < 8; ++bit) {
        SPI_CLK = 0;
        tmp = dat & 0x80;
        if (tmp == 0) {
            SPI_DIN = 0;
        } else {
            SPI_DIN = 1;
        }
        SPI_CLK = 1;
        dat <<= 1;
    }
    SPI_CS = 1;
    return ;
}

void SPI_SendCommand(unsigned char cmd) {
    unsigned char bit = 8, tmp = 0;
    SPI_DC = 0;
    SPI_CS = 0;

    for (bit = 0; bit < 8; ++bit) {
        SPI_CLK = 0;
        tmp = cmd & 0x80;
        if (tmp == 0) {
            SPI_DIN = 0;
        } else {
            SPI_DIN = 1;
        }
        SPI_CLK = 1;
        cmd <<= 1;
    }
    SPI_CS = 1;
    return ;
}


void SPI_Reset(void) {
    SPI_CS = 1;
    SPI_RST = 1;
    SPI_Delay_ms(10);
    SPI_RST = 0;
    SPI_Delay_ms(2);
    SPI_RST = 1;
    SPI_Delay_ms(10);
}

void SPI_ReadBusy(void) {
    UBYTE busy;
    do {
        SPI_SendCommand(0x71);
        busy = SPI_BUSY;
        busy = !(busy & 0x01);
    } while(busy);
    SPI_Delay_ms(200);
}

void SPI_TurnOnDisplay(void) {
    SPI_SendCommand(0x12);
    SPI_Delay_ms(100);
    SPI_ReadBusy();
}

void SPI_Init(void) {
    P0SEL = 0;
    P0DIR = 0;

    P1SEL = 0xfc;
    P1DIR = 0xbc;

    SPI_CLK = 1;
    SPI_RST = 0;
    SPI_Delay_ms(50);
    //
    SPI_SendCommand(0x04);
    SPI_ReadBusy();
    SPI_SendCommand(0x00);
    SPI_SendData(0x0f);
    SPI_SendData(0x89);
    SPI_SendCommand(0x61);
    SPI_SendData(0x68);
    SPI_SendData(0x00);
    SPI_SendData(0xd4);
    SPI_SendCommand(0x50);
    SPI_SendData(0x77);
}

void SPI_Clear(void) {
    UWORD width = (EPD_2IN9B_V3_WIDTH % 8 == 0) ? (EPD_2IN9B_V3_WIDTH / 8) :
        (EPD_2IN9B_V3_WIDTH/8 + 1);
    UWORD height = EPD_2IN9B_V3_HEIGHT;

    SPI_SendCommand(0x10);
    for (UWORD j = 0; j < height; ++j) {
        for (UWORD i = 0; i < width; ++i) {
            SPI_SendData(0xff);
        }
    }
    SPI_SendCommand(0x13);
    for (UWORD j = 0; j < height; ++j) {
        for (UWORD i = 0; i < width; ++i) {
            SPI_SendData(0xff);
        }
    }
    SPI_TurnOnDisplay();
}

void SPI_Display(const UBYTE *blackimage, const UBYTE *redimage) {
    UWORD width, height;
    width = (EPD_2IN9B_V3_WIDTH % 8 == 0) ? (EPD_2IN9B_V3_WIDTH / 8) :
        (EPD_2IN9B_V3_WIDTH/8 + 1);
    height = EPD_2IN9B_V3_HEIGHT;

    SPI_SendCommand(0x10);
    for (UWORD j = 0; j < height; ++j) {
        for (UWORD i = 0; i < width; ++i) {
            SPI_SendData(blackimage[i + j * width]);
        }
    }

    SPI_SendCommand(0x13);
    for (UWORD j = 0; j < height; ++j) {
        for (UWORD i = 0; i < width; ++i) {
            SPI_SendData(redimage[i + j * width]);
        }
    }

    SPI_TurnOnDisplay();
}

void SPI_Sleep(void) {
    SPI_SendCommand(0x50);
    SPI_SendData(0xf7);
    SPI_SendCommand(0x02);
    SPI_ReadBusy();
    SPI_SendCommand(0x07);
    SPI_SendData(0xa5);
}



#endif  /*  !__CC2530_DISPLAY_H__!  */