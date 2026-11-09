#include "ts_lcd.h"

// an initiallization function
// add description here
void ts_lcd_init(void) {
    adc_init();
    tft_init_hw();
    tft_begin();
}

// reads the status of the touchscreen
// returns true when the screen is touched
// assigns the current position of the touch to the px and py parameters
bool get_ts_lcd(uint16_t *px, uint16_t *py, uint16_t *pz) {
    struct TSPoint p;
    int x;
    int y;
    int z;

    // reads the status of the touchscreen
    getPoint(&p); 
    *pz = p.z;
    
    // checks if screen is actually being touched
    if (p.z > 500) {
        return false;
    }

    // perform linear interpolation NEEDS TO BE FIXED, IS HORRIBLY BROKEN


    //x = (p.x - 400) * 319 / (3700 - 400);
    //y = (p.y - 400) * 239 / (3700 - 400);

    uint32_t x1_screen = 200;
    uint32_t x2_screen = 3800;
    uint32_t x1_lcd = 0;
    uint32_t x2_lcd = 319;
    x = ((p.x - x1_screen) * (x2_lcd - x1_lcd)) / (x2_screen - x1_screen) + x1_lcd;

    uint32_t y1_screen = 200;
    uint32_t y2_screen = 3800;
    uint32_t y1_lcd = 0;
    uint32_t y2_lcd = 239;
    y = ((p.y - y1_screen) * (y2_lcd - y1_lcd)) / (y2_screen - y1_screen) + y1_lcd;
   
    // disregard values that are out of bounds
    /*
    if (x < 0) {
        x = 0;
    } else if (x > 319) {
        x = 319;
    }
    if (y < 0) {
        y = 0;
    } else if (y > 239) {
        y = 239;
    }
        */
    
    // assigns the current position of the touch to the px and py parameters
    *px = x;
    *py = y;
    
    return true;
}