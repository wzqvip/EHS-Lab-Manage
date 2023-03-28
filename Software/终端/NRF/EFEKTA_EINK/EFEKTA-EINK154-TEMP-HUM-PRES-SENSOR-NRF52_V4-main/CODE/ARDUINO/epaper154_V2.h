/*****************************************************************************
* | File      	:   epd1in54_V2.h
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

#ifndef EPAPER154_V2_H
#define EPAPER154_V2_H

#include "epaper.h"

// Display resolution
#define EPD_WIDTH       200
#define EPD_HEIGHT      200

class Epd : EpdIf {
public:
    unsigned long width;
    unsigned long height;

    Epd();
    ~Epd();
    int  Init(void);
    void SendCommand(unsigned char command);
    void SendData(unsigned char data);
	void SetWindows(int x_start, int y_start, int x_end, int y_end);
	void SetCursor(int x, int y);
    void WaitUntilIdle(void);
    void Reset(void);
    void Clear(void);
	void Clear2(void);
	void Clear3(void);
	void Clear4(void);
    void Display(const unsigned char* frame_buffer);
    void DisplayPartBaseImage(const unsigned char* frame_buffer);
    void DisplayPart(const unsigned char* frame_buffer);
	void DisplayWindows(const unsigned char* frame_buffer, unsigned int Xstart, unsigned int Ystart, unsigned int Xend, unsigned int Yend);
	void DisplayWindows2(const unsigned char* frame_buffer, unsigned int Xstart, unsigned int Ystart, unsigned int Xend, unsigned int Yend);
    
    void Sleep(void);
private:
    unsigned int reset_pin;
    unsigned int dc_pin;
    unsigned int cs_pin;
    unsigned int busy_pin;
};

#endif /* EPAPER154_V2_H */

/* END OF FILE */
