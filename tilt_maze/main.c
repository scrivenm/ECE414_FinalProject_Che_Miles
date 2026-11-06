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
        print("MPU6050 connection failed!\n");
        while (true){
            sleep_ms(1000);
        }
    }

    print("MPU6050 connected!\n");

    while(true){
        uint8_t reg = 0x3B; // Starting register for accelerometer data
        uint8_t data[6];

        int sent = i2c_write_blocking(i2c0, MPU_ADDR, &reg, 1, true);

        if (sent != 1){
            print("Failed to request data from MPU6050\n");
            sleep_ms(200);
            continue;
        }

        int received = i2c_read_blocking(i2c0, MPU_ADDR, data, 6, false);

        if (received == 6){
            int16_t ax = int16_t(data[0] << 8 | data[1]);
            int16_t ay = int16_t(data[2] << 8 | data[3]);
            int16_t az = int16_t(data[4] << 8 | data[5]);
            print("Accelerometer Data - X: %d, Y: %d, Z: %d\n", ax, ay, az);
        } else {
            print("Failed to read data from MPU6050\n");
        }

        sleep_ms(200);
    }
}