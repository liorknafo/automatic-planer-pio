#pragma once
#include "motor.hpp"
#include "encoder.hpp"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"

class MotionSystem
{
private:
    Motor &motorX;
    Motor &motorY;

    Encoder yEncoder;

    gpio_num_t yStartLimit;
    gpio_num_t yEndLimit;
    gpio_num_t xStartLimit;
    gpio_num_t xEndLimit;
    
    TaskHandle_t move_task_handle;
    TaskHandle_t home_task_handle;
    
    void move_task();
    void home_task();

    void move_x_to_end();
    void move_x_to_start();
    bool move_y_steps();

public:

    float cutting_speed;
    float travel_speed;
    uint32_t y_offset;

    MotionSystem(Motor &motorX, Motor &motorY,gpio_num_t yEncoder, gpio_num_t yStartLimit, gpio_num_t yEndLimit, gpio_num_t xStartLimit, gpio_num_t xEndLimit);
    
    void init();
    void move();
    void stop();
    void home();
};