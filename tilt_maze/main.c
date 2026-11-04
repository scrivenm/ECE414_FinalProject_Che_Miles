#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#define MPU_Addr 0x68
#define SDA_PIN 4
#define SCL_PIN 5

int main(){
    stdio_init_all();
    sleep_ms(2000);

    // Set up I2C
    i2c_init(i2c0, 100000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    // Wake up the MPU6050
    uint8_t wake(2) ={0x68, 0x00};
    int result = i2c_write_blocking(
        i2c0, MPU_ADDR, wake, 2, false
    );

    if (result != 2){
        print("MPU6050 conenction failed!\n");
        while (true){
            sleep_ms(1000);
        }
    }

    print("MPU6050 connected!\n");
}