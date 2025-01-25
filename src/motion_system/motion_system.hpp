#pragma once
// C++ Standard Library
#include <memory>

// ESP-IDF
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Project Headers
#include "motor.hpp"
#include "encoder.hpp"
#include "bounded_number.hpp"
#include "control_panel/joystick.hpp"
#include "switch.hpp"

class MotionSystem
{
private:
    Motor &motorX;
    Motor &motorY;
    Encoder yEncoder;

    Switch yStartLimit;
    Switch yEndLimit;
    Switch xStartLimit;
    Switch xEndLimit;
    
    TaskHandle_t move_task_handle;
    TaskHandle_t home_task_handle;
    TaskHandle_t move_by_joystick_task_handle;

    std::shared_ptr<BoundedNumber<float>> cutting_speed;
    std::shared_ptr<BoundedNumber<float>> travel_speed;
    std::shared_ptr<BoundedNumber<uint32_t>> y_offset;
    
    Joystick &joystick;

    void move_task();
    void home_task();
    void move_by_joystick_task();

    void move_x_to_end();
    void move_x_to_start();
    bool move_y_steps();

public:
    MotionSystem(Motor &motorX, Motor &motorY, 
                 gpio_num_t yEncoder, gpio_num_t yStartLimit, gpio_num_t yEndLimit, 
                 gpio_num_t xStartLimit, gpio_num_t xEndLimit,
                 std::shared_ptr<BoundedNumber<float>> cutting_speed,
                 std::shared_ptr<BoundedNumber<float>> travel_speed,
                 std::shared_ptr<BoundedNumber<uint32_t>> y_offset,
                 Joystick &joystick);
    ~MotionSystem();
    
    void move();
    void stop();
    void home();
    void move_by_joystick();
    void stop_move_by_joystick();
};