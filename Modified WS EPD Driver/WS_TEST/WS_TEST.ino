#include "src/e-Paper/EPD_2in13_V4.h"
#include "src/Examples/EPD_Test.h"

void setup() {
    // EPD_2in13_V4_test();
    // EPD_2in13b_V3_test();
    // EPD_2in13b_V4_test();
}


void loop() {
    EPD_2in13_V4_test();
    
    // DEV_Module_Init();
    // UBYTE *BlackImage;
    // UWORD Imagesize = ((EPD_2in13_V4_WIDTH % 8 == 0)? (EPD_2in13_V4_WIDTH / 8 ): (EPD_2in13_V4_WIDTH / 8 + 1)) * EPD_2in13_V4_HEIGHT;
    
	// EPD_2in13_V4_Init();
    // EPD_2in13_V4_Clear();
    // Paint_NewImage(BlackImage, EPD_2in13_V4_WIDTH, EPD_2in13_V4_HEIGHT, 90, WHITE);
    // Paint_Clear(WHITE);
    // DEV_Delay_ms(2000);

    // EPD_2in13_V4_Init_Fast();
    // Paint_SelectImage(BlackImage);
    // Paint_Clear(WHITE);
    // Paint_DrawBitMap(gImage_2in13);

    // EPD_2in13_V4_Display_Fast(BlackImage);
    // DEV_Delay_ms(2000);

    // Paint_NewImage(BlackImage, EPD_2in13_V4_WIDTH, EPD_2in13_V4_HEIGHT, 90, WHITE);  	
    // Debug("Drawing\r\n");
    // //1.Select Image
    // EPD_2in13_V4_Init();
    // Paint_SelectImage(BlackImage);
    // Paint_Clear(WHITE);

    // Paint_DrawPoint(5, 10, BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT);
    // Paint_DrawPoint(5, 25, BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
    // Paint_DrawPoint(5, 40, BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);
    // Paint_DrawPoint(5, 55, BLACK, DOT_PIXEL_4X4, DOT_STYLE_DFT);
    // EPD_2in13_V4_Display_Base(BlackImage);
    // DEV_Delay_ms(3000);
}