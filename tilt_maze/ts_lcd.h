#ifndef TS_LCD_H
#define TS_LCD_H

#include <stdbool.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "TFTMaster.h"
#include "TouchScreen.h"

// an initiallization function
// add description here
void ts_lcd_init(void);

// reads the status of the touchscreen
// returns true when the screen is touched
// assigns the current position of the touch to the px and py parameters
bool get_ts_lcd(uint16_t *px, uint16_t *py, uint16_t *pz);

#endif