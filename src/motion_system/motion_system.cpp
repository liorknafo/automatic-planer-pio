#include "motion_system.hpp"
#include "freertos/FreeRTOS.h"
#include <iostream>

void MotionSystem::move_x_to_end()
{
    this->motorX.move_forward(this->cutting_speed);
    while (1)
    {
        if (gpio_get_level(this->xEndLimit) == 0)
        {
            this->motorX.stop();
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void MotionSystem::move_x_to_start()
{
    this->motorX.move_backward(this->cutting_speed);
    while (1)
    {
        if (gpio_get_level(this->xStartLimit) == 0)
        {
            this->motorX.stop();
            return;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

bool MotionSystem::move_y_steps()
{
    this->yEncoder.reset();
    this->motorY.move_forward(this->cutting_speed);
    while (1)
    {
        if (gpio_get_level(this->yEndLimit) == 0)
        {
            this->motorY.stop();
            return true;
        }
        else if (this->yEncoder.read() >= this->y_offset)
        {
            this->motorY.stop();
            return false;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void MotionSystem::move_task()
{
    while (true)
    {
        move_x_to_end();
        if (move_y_steps())
            break;
        move_x_to_start();
        if (move_y_steps())
            break;
        ;
    }
    this->move_task_handle = nullptr;
    vTaskDelete(nullptr);
}

void MotionSystem::home_task()
{
    std::cout << "start home" << std::endl;
    this->motorX.move_backward(this->travel_speed);
    this->motorY.move_backward(this->travel_speed);
    std::cout << "moving" << std::endl;
    while (1)
    {
        bool xDone = gpio_get_level(this->xStartLimit) == 0;
        std::cout << "x: " << xDone << std::endl;
        if (xDone)
            this->motorX.stop();
        bool yDone = gpio_get_level(this->yStartLimit) == 0;
        std::cout << "y: " << yDone << std::endl;
        if (yDone)
            this->motorY.stop();
        if (xDone && yDone)
            break;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    std::cout << "done home" << std::endl;
    this->home_task_handle = nullptr;
    vTaskDelete(nullptr);
}

MotionSystem::MotionSystem(Motor &motorX, Motor &motorY, gpio_num_t yEncoder, gpio_num_t yStartLimit, gpio_num_t yEndLimit, gpio_num_t xStartLimit, gpio_num_t xEndLimit) : motorX(motorX),
                                                                                                                                                                            motorY(motorY),
                                                                                                                                                                            yEncoder(yEncoder),
                                                                                                                                                                            yStartLimit(yStartLimit),
                                                                                                                                                                            yEndLimit(yEndLimit),
                                                                                                                                                                            xStartLimit(xStartLimit),
                                                                                                                                                                            xEndLimit(xEndLimit),
                                                                                                                                                                            move_task_handle(nullptr),
                                                                                                                                                                            home_task_handle(nullptr),
                                                                                                                                                                            cutting_speed(0.5),
                                                                                                                                                                            travel_speed(1),
                                                                                                                                                                            y_offset(3)
{
}

void MotionSystem::init()
{
    this->yEncoder.init();
    this->motorX.init();
    this->motorY.init();

    gpio_config_t io_conf = {
        .pin_bit_mask = (uint64_t)(1 << (this->yStartLimit) |
                                   1 << (this->yEndLimit) |
                                   1 << (this->xStartLimit) |
                                   1 << (this->xEndLimit)),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);
}

void MotionSystem::move()
{
    if (this->move_task_handle != nullptr || this->home_task_handle != nullptr)
        return;
    xTaskCreate([](void *param)
                { ((MotionSystem *)param)->move_task(); }, "motion_system_move_task", 8096, this, 5, &this->move_task_handle);
}

void MotionSystem::stop()
{
    this->motorY.stop();
    this->motorX.stop();
    if (this->move_task_handle != nullptr)
        vTaskDelete(this->move_task_handle);
    if (this->home_task_handle != nullptr)
        vTaskDelete(this->home_task_handle);
}

void MotionSystem::home()
{
    if (this->move_task_handle != nullptr || this->home_task_handle != nullptr)
        return;
    xTaskCreate([](void *param)
                { 
                    ((MotionSystem *)param)->home_task(); }, "motion_system_home_task", 8096, this, 5, &this->home_task_handle);
}