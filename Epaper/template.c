#include <ioCC2530.h>
#include "types.h"
#include "display.h"
// #include "GUI_Paint.h"
// #include "ImageData.h"

#define Imagesize ((EPD_2IN9B_V3_WIDTH / 8 + 1) * EPD_2IN9B_V3_HEIGHT)

//UBYTE BlackImage [Imagesize];
//UBYTE RYImage [Imagesize];

int main(void) {
    
    SPI_Init();
    SPI_Clear();
    SPI_Delay_ms(5);
  //  SPI_Display(BlackImage, BlackImage);
    SPI_Delay_ms(2000);

    SPI_Clear();
    SPI_Sleep();
    SPI_Delay_ms(2000);
    return (0);
}
