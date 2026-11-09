#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/i2c.h"

#define MPU_ADDR 0x68
#define SDA_PIN 4
#define SCL_PIN 5

#define LCD_WIDTH  240
#define LCD_HEIGHT 320
#define BALL_RADIUS 8

#define PIN_CS   17
#define PIN_DC   16
#define PIN_SCK  18
#define PIN_MOSI 19
#define PIN_RST  20

// color def
#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF
#define COLOR_RED   0xF800
#define COLOR_BLUE  0x001F

// --- ILI9341 屏幕驱动函数 ---
void lcd_write_cmd(uint8_t cmd) {
    gpio_put(PIN_DC, 0); 
    gpio_put(PIN_CS, 0);
    spi_write_blocking(spi0, &cmd, 1);
    gpio_put(PIN_CS, 1);
}

void lcd_write_data(uint8_t data) {
    gpio_put(PIN_DC, 1);
    gpio_put(PIN_CS, 0);
    spi_write_blocking(spi0, &data, 1);
    gpio_put(PIN_CS, 1);
}

void ili9341_init() {
    gpio_put(PIN_RST, 0);
    sleep_ms(50);
    gpio_put(PIN_RST, 1);
    sleep_ms(120);

    lcd_write_cmd(0x01);
    sleep_ms(120);

    lcd_write_cmd(0x11); 
    sleep_ms(120);

    lcd_write_cmd(0x3A); 
    lcd_write_data(0x55);

    lcd_write_cmd(0x29); 
}

void lcd_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    lcd_write_cmd(0x2A); // line address
    lcd_write_data(x0 >> 8); lcd_write_data(x0 & 0xFF);
    lcd_write_data(x1 >> 8); lcd_write_data(x1 & 0xFF);

    lcd_write_cmd(0x2B); // row address
    lcd_write_data(y0 >> 8); lcd_write_data(y0 & 0xFF);
    lcd_write_data(y1 >> 8); lcd_write_data(y1 & 0xFF);

    lcd_write_cmd(0x2C); // ram

void lcd_draw_circle_filled(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    for (int16_t y = -r; y <= r; y++) {
        for (int16_t x = -r; x <= r; x++) {
            if (x * x + y * y <= r * r) {
                int16_t px = x0 + x;
                int16_t py = y0 + y;
                if (px >= 0 && px < LCD_WIDTH && py >= 0 && py < LCD_HEIGHT) {
                    lcd_set_window(px, py, px, py);
                    gpio_put(PIN_DC, 1);
                    gpio_put(PIN_CS, 0);
                    uint8_t bytes[] = {color >> 8, color & 0xFF};
                    spi_write_blocking(spi0, bytes, 2);
                    gpio_put(PIN_CS, 1);
                }
            }
        }
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000);

    // connect MPU6050
    i2c_init(i2c0, 100000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    // MPU6050 wake up
    uint8_t wake[2] = {0x6B, 0x00};
    int result = i2c_write_blocking(i2c0, MPU_ADDR, wake, 2, false);
    if (result != 2) {
        printf("MPU6050 connection failed!\n");
    } else {
        printf("MPU6050 connected!\n");
    }

    // initialize the screen
    spi_init(spi0, 30 * 1000 * 1000); // 30MHz SPI
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    gpio_init(PIN_CS);  gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_init(PIN_DC);  gpio_set_dir(PIN_DC, GPIO_OUT);
    gpio_init(PIN_RST); gpio_set_dir(PIN_RST, GPIO_OUT);

    ili9341_init();

    // initialize the place of the little ball
    float ball_x = LCD_WIDTH / 2.0f;
    float ball_y = LCD_HEIGHT / 2.0f;
    float ball_vx = 0.0f, ball_vy = 0.0f;
    
    int16_t prev_x = (int16_t)ball_x;
    int16_t prev_y = (int16_t)ball_y;
    const float dt = 0.02f; // refresh T

    while (true) {
        // read MPU6050 data
        uint8_t reg = 0x3B;//
        uint8_t data[6];

        int sent = i2c_write_blocking(i2c0, MPU_ADDR, &reg, 1, true);
        if (sent == 1) {
            int received = i2c_read_blocking(i2c0, MPU_ADDR, data, 6, false);
            if (received == 6) {
                int16_t ax = (int16_t)(data[0] << 8 | data[1]);
                int16_t ay = (int16_t)(data[2] << 8 | data[3]);
                
                // accleration of the ball we can adjust later
                float ax_f = (float)ax / 16384.0f; // 
                float ay_f = (float)ay / 16384.0f;

                // refresh place of ball
                ball_vx += ay_f * dt * 150.0f; 
                ball_vy += ax_f * dt * 150.0f;

                ball_vx *= 0.92f;
                ball_vy *= 0.92f;

                ball_x += ball_vx;
                ball_y += ball_vy;

                // boundry of the maze, now is a big square
                if (ball_x < BALL_RADIUS) { ball_x = BALL_RADIUS; ball_vx = -ball_vx * 0.3f; }
                if (ball_x > LCD_WIDTH - BALL_RADIUS) { ball_x = LCD_WIDTH - BALL_RADIUS; ball_vx = -ball_vx * 0.3f; }
                if (ball_y < BALL_RADIUS) { ball_y = BALL_RADIUS; ball_vy = -ball_vy * 0.3f; }
                if (ball_y > LCD_HEIGHT - BALL_RADIUS) { ball_y = LCD_HEIGHT - BALL_RADIUS; ball_vy = -ball_vy * 0.3f; }
            }
        }

        // redraw the litel ball
        int16_t curr_x = (int16_t)ball_x;
        int16_t curr_y = (int16_t)ball_y;

        if (curr_x != prev_x || curr_y != prev_y) {
            lcd_draw_circle_filled(prev_x, prev_y, BALL_RADIUS, COLOR_BLACK); // old graph
            lcd_draw_circle_filled(curr_x, curr_y, BALL_RADIUS, COLOR_RED);   // new graph
            prev_x = curr_x;
            prev_y = curr_y;
        }

        sleep_ms(20); 
    }
    return 0;
}
