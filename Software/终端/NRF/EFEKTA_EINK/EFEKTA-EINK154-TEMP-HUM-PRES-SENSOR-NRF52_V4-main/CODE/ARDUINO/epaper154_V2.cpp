/*****************************************************************************
* | File      	:   epd1in54_V2.cpp
* | Author      :   Waveshare team
* | Function    :   1.54inch e-paper V2
* | Info        :
*----------------
* |	This version:   V1.0
* | Date        :   2019-06-24
* | Info        :
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documnetation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to  whom the Software is
# furished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
******************************************************************************/
#include <stdlib.h>
#include "epaper154_V2.h"

Epd::~Epd()
{
};

Epd::Epd()
{
    reset_pin = RST_PIN;
    dc_pin = DC_PIN;
    cs_pin = CS_PIN;
    busy_pin = BUSY_PIN;
    width = EPD_WIDTH;
    height = EPD_HEIGHT;
};

/**
 *  @brief: basic function for sending commands
 */
void Epd::SendCommand(unsigned char command)
{
    DigitalWrite(dc_pin, LOW);
    SpiTransfer(command);
}

/**
 *  @brief: basic function for sending data
 */
void Epd::SendData(unsigned char data)
{
    DigitalWrite(dc_pin, HIGH);
    SpiTransfer(data);
}

/**
 *  @brief: Wait until the busy_pin goes HIGH
 */
void Epd::WaitUntilIdle(void)
{
    while(DigitalRead(busy_pin) == 1) {      //LOW: idle, HIGH: busy
        DelayMs(50);
    }
}

/**
    @brief: private function to specify the memory area for data R/W
*/
void Epd::SetWindows(int x_start, int y_start, int x_end, int y_end)
{
    SendCommand(0x44);
    /* x point must be the multiple of 8 or the last 3 bits will be ignored */
    SendData((x_start >> 3) & 0xFF);
    SendData((x_end >> 3) & 0xFF);
    SendCommand(0x45);
    SendData(y_start & 0xFF);
    SendData((y_start >> 8) & 0xFF);
    SendData(y_end & 0xFF);
    SendData((y_end >> 8) & 0xFF);
}

/**
    @brief: private function to specify the start point for data R/W
*/
void Epd::SetCursor(int x, int y)
{
    SendCommand(0x4E);
    /* x point must be the multiple of 8 or the last 3 bits will be ignored */
    SendData((x >> 3) & 0xFF);
    SendCommand(0X4F);
    SendData(y & 0xFF);
    SendData((y >> 8) & 0xFF);
    WaitUntilIdle();
}

int Epd::Init(void)
{
    /* this calls the peripheral hardware interface, see epdif */
    if (IfInit() != 0) {
        return -1;
    }
    /* EPD hardware init start */
    Reset();

    WaitUntilIdle();
    SendCommand(0x12);  //SWRESET
    WaitUntilIdle();

    SendCommand(0x01); //Driver output control
    SendData(0xC7);
    SendData(0x00);
    SendData(0x01);

    SendCommand(0x11); //data entry mode
    SendData(0x01);

    SendCommand(0x44); //set Ram-X address start/end position
    SendData(0x00);
    SendData(0x18);    //0x0C-->(18+1)*8=200

    SendCommand(0x45); //set Ram-Y address start/end position
    SendData(0xC7);   //0xC7-->(199+1)=200
    SendData(0x00);
    SendData(0x00);
    SendData(0x00);

    SendCommand(0x3C); //BorderWavefrom
    SendData(0x01);

    SendCommand(0x18);
    SendData(0x80);

    SendCommand(0x22); // //Load Temperature and waveform setting.
    SendData(0XB1);
    SendCommand(0x20);

    SendCommand(0x4E);   // set RAM x address count to 0;
    SendData(0x00);
    SendCommand(0x4F);   // set RAM y address count to 0X199;
    SendData(0xC7);
    SendData(0x00);
    WaitUntilIdle();
    /* EPD hardware init end */

    return 0;
}

/**
 *  @brief: module reset.
 *          often used to awaken the module in deep sleep,
 *          see Epd::Sleep();
 */
void Epd::Reset(void)
{
    DigitalWrite(reset_pin, HIGH);
    DelayMs(100);
    DigitalWrite(reset_pin, LOW);                //module reset
    DelayMs(10);
    DigitalWrite(reset_pin, HIGH);
    DelayMs(100);
}

void Epd::Clear(void)
{
    int w, h;
    w = (EPD_WIDTH % 8 == 0)? (EPD_WIDTH / 8 ): (EPD_WIDTH / 8 + 1);
    h = EPD_HEIGHT;
    SendCommand(0x24);
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            SendData(0xff);
        }
    }
	
	SendCommand(0x26);
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            SendData(0xff);
        }
    }

    //DISPLAY REFRESH
    SendCommand(0x22);
    SendData(0xFF);
    SendCommand(0x20);
    WaitUntilIdle();
}

void Epd::Clear3(void)
{
    int w, h;
    w = (EPD_WIDTH % 8 == 0)? (EPD_WIDTH / 8 ): (EPD_WIDTH / 8 + 1);
    h = EPD_HEIGHT;
    SendCommand(0x26);
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            SendData(0x00);
        }
    }
	
	SendCommand(0x24);
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            SendData(0xff);
        }
    }

    //DISPLAY REFRESH
    SendCommand(0x22);
    SendData(0xFF);
    SendCommand(0x20);
    WaitUntilIdle();
}

void Epd::Clear4(void)
{
    int w, h;
    w = (EPD_WIDTH % 8 == 0)? (EPD_WIDTH / 8 ): (EPD_WIDTH / 8 + 1);
    h = EPD_HEIGHT;
  
	
	SendCommand(0x24);
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            SendData(0xff);
        }
    }

    //DISPLAY REFRESH
    SendCommand(0x22);
    SendData(0xFF);
    SendCommand(0x20);
    WaitUntilIdle();
}

void Epd::Clear2(void)
{
    int w, h;
    w = (EPD_WIDTH % 8 == 0)? (EPD_WIDTH / 8 ): (EPD_WIDTH / 8 + 1);
    h = EPD_HEIGHT;
    SendCommand(0x26);
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            SendData(0x00);
        }
    }
	SendCommand(0x24);
    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            SendData(0xFF);
        }
    }

    //DISPLAY REFRESH
    SendCommand(0x22);
    SendData(0xF7);
	//SendData(0xFF);
    SendCommand(0x20);
    WaitUntilIdle();
}

void Epd::Display(const unsigned char* frame_buffer)
{
    int w = (EPD_WIDTH % 8 == 0)? (EPD_WIDTH / 8 ): (EPD_WIDTH / 8 + 1);
    int h = EPD_HEIGHT;

    if (frame_buffer != NULL) {
        SendCommand(0x24);
        for (int j = 0; j < h; j++) {
            for (int i = 0; i < w; i++) {
                SendData(pgm_read_byte(&frame_buffer[i + j * w]));
            }
        }
    }

    //DISPLAY REFRESH
    //SendCommand(0x22);
    //SendData(0xFF);
    //SendCommand(0x20);
    //WaitUntilIdle();
}

void Epd::DisplayPartBaseImage(const unsigned char* frame_buffer)
{
    int w = (EPD_WIDTH % 8 == 0)? (EPD_WIDTH / 8 ): (EPD_WIDTH / 8 + 1);
    int h = EPD_HEIGHT;

    if (frame_buffer != NULL) {
        SendCommand(0x24);
        for (int j = 0; j < h; j++) {
            for (int i = 0; i < w; i++) {
                SendData(frame_buffer[i + j * w]);
            }
        }

        SendCommand(0x26);
        for (int j = 0; j < h; j++) {
            for (int i = 0; i < w; i++) {
                SendData(frame_buffer[i + j * w]);
            }
        }
    }

    //DISPLAY REFRESH
    SendCommand(0x22);
    SendData(0xFF);
    SendCommand(0x20);
    WaitUntilIdle();
}

void Epd::DisplayPart(const unsigned char* frame_buffer)
{
    int w = (EPD_WIDTH % 8 == 0)? (EPD_WIDTH / 8 ): (EPD_WIDTH / 8 + 1);
    int h = EPD_HEIGHT;

    if (frame_buffer != NULL) {
        SendCommand(0x24);
        for (int j = 0; j < h; j++) {
            for (int i = 0; i < w; i++) {
                SendData(frame_buffer[i + j * w]);
            }
        }
    }

    //DISPLAY REFRESH
    SendCommand(0x22);
    SendData(0xFF);
    SendCommand(0x20);
    WaitUntilIdle();
}

void Epd::DisplayWindows2(const unsigned char* frame_buffer, unsigned int Xstart, unsigned int Ystart, unsigned int Xend, unsigned int Yend)
{
    unsigned int Width, Height;
    Width = ((Xend - Xstart) % 8 == 0) ? ((Xend - Xstart) / 8 ) : ((Xend - Xstart) / 8 + 1);
    Height = Yend - Ystart;

    unsigned int i, j;
	for (j = 0; j < Height; j++) {
        SetCursor(Xstart, Ystart + j);
        SendCommand(0x26);
        for (i = 0; i < Width; i++) {
            SendData(frame_buffer[i + j * Width]);
        }
    }
    for (j = 0; j < Height; j++) {
        SetCursor(Xstart, Ystart + j);
        SendCommand(0x24);
        for (i = 0; i < Width; i++) {
            SendData(~frame_buffer[i + j * Width]);
        }
    }
}

void Epd::DisplayWindows(const unsigned char* frame_buffer, unsigned int Xstart, unsigned int Ystart, unsigned int Xend, unsigned int Yend)
{
    unsigned int Width, Height;
    Width = ((Xend - Xstart) % 8 == 0) ? ((Xend - Xstart) / 8 ) : ((Xend - Xstart) / 8 + 1);
    Height = Yend - Ystart;

    unsigned int i, j;
	for (j = 0; j < Height; j++) {
        SetCursor(Xstart, Ystart + j);
        SendCommand(0x24);
        for (i = 0; i < Width; i++) {
            SendData(frame_buffer[i + j * Width]);
        }
    }
}

/*
void Epd::DisplayWindows2(unsigned int x_start,unsigned int y_start,const unsigned char * datas,unsigned int PART_COLUMN,unsigned int PART_LINE)
{
  unsigned int i;  
  unsigned int x_end,y_start1,y_start2,y_end1,y_end2;
  x_start=x_start/8;//
  x_end=x_start+PART_LINE/8-1; 
  
  y_start1=0;
  y_start2=y_start;
  if(y_start>=256)
  {
    y_start1=y_start2/256;
    y_start2=y_start2%256;
  }
  y_end1=0;
  y_end2=y_start+PART_COLUMN-1;
  if(y_end2>=256)
  {
    y_end1=y_end2/256;
    y_end2=y_end2%256;    
  }   
  
  Epaper_Write_Command(0x44);       // set RAM x address start/end, in page 35
  Epaper_Write_Data(x_start);    // RAM x address start at 00h;
  Epaper_Write_Data(x_end);    // RAM x address end at 0fh(15+1)*8->128 
  Epaper_Write_Command(0x45);       // set RAM y address start/end, in page 35
  Epaper_Write_Data(y_start2);    // RAM y address start at 0127h;
  Epaper_Write_Data(y_start1);    // RAM y address start at 0127h;
  Epaper_Write_Data(y_end2);    // RAM y address end at 00h;
  Epaper_Write_Data(y_end1);    // ????=0 


  Epaper_Write_Command(0x4E);   // set RAM x address count to 0;
  Epaper_Write_Data(x_start); 
  Epaper_Write_Command(0x4F);   // set RAM y address count to 0X127;    
  Epaper_Write_Data(y_start2);
  Epaper_Write_Data(y_start1);
  
  
   Epaper_Write_Command(0x24);   //Write Black and White image to RAM
   for(i=0;i<PART_COLUMN*PART_LINE/8;i++)
   {                         
     Epaper_Write_Data(pgm_read_byte(&datas[i]));
   } 
   EPD_Part_Update();

}
*/





/**
 *  @brief: After this command is transmitted, the chip would enter the
 *          deep-sleep mode to save power.
 *          The deep sleep mode would return to standby by hardware reset.
 *          The only one parameter is a check code, the command would be
 *          executed if check code = 0xA5.
 *          You can use Epd::Init() to awaken
 */
void Epd::Sleep()
{
    SendCommand(0x10); //enter deep sleep
    SendData(0x01);
    //DelayMs(200);

    //DigitalWrite(reset_pin, LOW);
}

/* END OF FILE */
