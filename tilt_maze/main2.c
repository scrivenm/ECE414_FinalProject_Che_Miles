#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "TFTMaster.h"
#include "ts_lcd.h"

// game state def
enum GameState {
    GAME_INIT,
    GAME_PLAYING,
    GAME_WIN,
    GAME_RESET } GameState;

// screen and ball setting
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320
#define BALL_RADIUS   6

// MPU6050
#define MPU_ADDR 0x68
#define SDA_PIN  4
#define SCL_PIN  5

// physical movement of the ball
static float ball_x = 30.0f;
static float ball_y = 30.0f;
static float ball_vx = 0.0f;
static float ball_vy = 0.0f;

static int16_t prev_x = 30;
static int16_t prev_y = 30;

// maze xyz
typedef struct {
    int16_t x, y, w, h; } Wall;

// maze map def
static const Wall maze_walls[] = {
    {0, 0, 240, 10},   // top
    {0, 0, 10, 320},   // left
    {230, 0, 10, 320}, // right
    {0, 310, 240, 10}, // down
    {40, 50, 150, 10}, // inner wall
    {40, 120, 10, 140},// inner wall
    {110, 120, 90, 10} // inner wall
};
#define NUM_WALLS (sizeof(maze_walls) / sizeof(Wall))

// draw the wall and maze
void draw_maze_screen() {
    tft_fillScreen(ILI9340_BLACK);
    
    // blue boundry of the maze
    for (int i = 0; i < NUM_WALLS; i++) {
        tft_fillRect(maze_walls[i].x, maze_walls[i].y, maze_walls[i].w, maze_walls[i].h, ILI9340_BLUE);
    }
    
    // the final place need to go
    tft_fillRect(190, 270, 30, 30, ILI9340_GREEN);
}

// collision detection of the ball
bool check_collision(float x, float y, float r) {
    for (int i = 0; i < NUM_WALLS; i++) {
        if (x + r > maze_walls[i].x && x - r < maze_walls[i].x + maze_walls[i].w &&
            y + r > maze_walls[i].y && y - r < maze_walls[i].y + maze_walls[i].h) {
            return true;
        }
    }
    return false;
}

// I use something similar to our calculator tick
void maze_game_tick(float ax, float ay) {
    const float dt = 0.02f; // 20ms 周期

    switch (GameState) {
        case GAME_INIT:
            draw_maze_screen();
            ball_x = 25.0f;
            ball_y = 25.0f;
            ball_vx = 0.0f;
            ball_vy = 0.0f;
            GameState = GAME_PLAYING;
            break;

        case GAME_PLAYING:
            // update the MCU 6050 acceleration
            ball_vx += ay * dt * 140.0f; 
            ball_vy += ax * dt * 140.0f;

            ball_vx *= 0.92f;
            ball_vy *= 0.92f;

            // X axis rebound and collision
            float next_x = ball_x + ball_vx;
            if (!check_collision(next_x, ball_y, BALL_RADIUS)) {
                ball_x = next_x;
            } else {
                ball_vx = -ball_vx * 0.3f; 
            }

            // Y axis
            float next_y = ball_y + ball_vy;
            if (!check_collision(ball_x, next_y, BALL_RADIUS)) {
                ball_y = next_y;
            } else {
                ball_vy = -ball_vy * 0.3f;
            }

            // finish game setting
            if (ball_x >= 190 && ball_x <= 220 && ball_y >= 270 && ball_y <= 300) {
                GameState = GAME_WIN;
            }
            break;

        case GAME_WIN:
            tft_setCursor(50, 140);
            tft_setTextColor(ILI9340_YELLOW);
            tft_setTextSize(3);
            tft_writeString("YOU WIN!");
            break;

        case GAME_RESET:
            GameState = GAME_INIT;
            break;

        default:
            GameState = GAME_INIT;
            break;
    }
}

// to refresh the ball place
void render_ball() {
    int16_t curr_x = (int16_t)ball_x;
    int16_t curr_y = (int16_t)ball_y;

    if (curr_x != prev_x || curr_y != prev_y) {
        // erase the oled place
        tft_fillCircle(prev_x, prev_y, BALL_RADIUS, ILI9340_BLACK);
        
        // draw new place
        tft_fillCircle(curr_x, curr_y, BALL_RADIUS, ILI9340_RED);
        
        prev_x = curr_x;
        prev_y = curr_y;
    }
}

int main() {
    stdio_init_all();
    sleep_ms(1500);

    // initialize the screen
    ts_lcd_init();

    // initialize I2C conneting MPU6050
    i2c_init(i2c0, 100000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    // wake up MPU6050
    uint8_t wake[2] = {0x6B, 0x00};
    i2c_write_blocking(i2c0, MPU_ADDR, wake, 2, false);

    // initialize game state
    GameState = GAME_INIT;

    while (1) {
        // clk
        sleep_ms(20);

        // read acceleration date given by MPU6050
        uint8_t reg = 0x3B;
        uint8_t data[6];
        float ax = 0.0f, ay = 0.0f;

        if (i2c_write_blocking(i2c0, MPU_ADDR, &reg, 1, true) == 1) {
            if (i2c_read_blocking(i2c0, MPU_ADDR, data, 6, false) == 6) {
                int16_t raw_ax = (int16_t)(data[0] << 8 | data[1]);
                int16_t raw_ay = (int16_t)(data[2] << 8 | data[3]);
                ax = (float)raw_ax / 16384.0f;
                ay = (float)raw_ay / 16384.0f;
            }
        }

        // run game state
        maze_game_tick(ax, ay);

        // refresh picture
        if (GameState == GAME_PLAYING) {
            render_ball();
        }
    }
    return 0;
}
