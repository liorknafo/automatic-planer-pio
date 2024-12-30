#include "motion_system.hpp"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include <iostream>

void MotionSystem::move_x_to_end()
{
    this->motorX.move_forward(this->cutting_speed->get());
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
    this->motorX.move_backward(this->cutting_speed->get());
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
    this->motorY.move_forward(this->cutting_speed->get());
    while (1)
    {
        if (gpio_get_level(this->yEndLimit) == 0)
        {
            this->motorY.stop();
            return true;
        }
        else if (this->yEncoder.read() >= this->y_offset->get())
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
        this->move_x_to_end();
        if (this->move_y_steps())
            break;
        this->move_x_to_start();
        if (this->move_y_steps())
            break;
    }
    this->move_task_handle = nullptr;
    vTaskDelete(nullptr);
}

void MotionSystem::home_task()
{
    std::cout << "start home" << std::endl;
    float speed = this->travel_speed->get();
    this->motorX.move_backward(speed);
    this->motorY.move_backward(speed);
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

MotionSystem::MotionSystem(Motor &motorX, Motor &motorY,
                           gpio_num_t yEncoder, gpio_num_t yStartLimit, gpio_num_t yEndLimit,
                           gpio_num_t xStartLimit, gpio_num_t xEndLimit,
                           std::shared_ptr<BoundedNumber<float>> cutting_speed,
                           std::shared_ptr<BoundedNumber<float>> travel_speed,
                           std::shared_ptr<BoundedNumber<uint32_t>> y_offset)
    : motorX(motorX),
      motorY(motorY),
      yEncoder(yEncoder),
      yStartLimit(yStartLimit),
      yEndLimit(yEndLimit),
      xStartLimit(xStartLimit),
      xEndLimit(xEndLimit),
      move_task_handle(nullptr),
      home_task_handle(nullptr),
      cutting_speed(cutting_speed),
      travel_speed(travel_speed),
      y_offset(y_offset)
{
}

MotionSystem::~MotionSystem()
{
    stop(); // Ensure all tasks are stopped
}

void MotionSystem::init()
{
    this->yEncoder.init();
    this->motorX.init();
    this->motorY.init();

    gpio_config_t io_conf = {
        .pin_bit_mask = ((1ULL << this->yStartLimit) |
                         (1ULL << this->yEndLimit) |
                         (1ULL << this->xStartLimit) |
                         (1ULL << this->xEndLimit)),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));
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
    {
        TaskHandle_t move_handle = this->move_task_handle;
        this->move_task_handle = nullptr;
        vTaskDelete(move_handle);
    }
    if (this->home_task_handle != nullptr)
    {
        TaskHandle_t home_handle = this->home_task_handle;
        this->home_task_handle = nullptr;
        vTaskDelete(home_handle);
    }
}

void MotionSystem::home()
{
    if (this->move_task_handle != nullptr || this->home_task_handle != nullptr)
        return;
    xTaskCreate([](void *param)
                { ((MotionSystem *)param)->home_task(); }, "motion_system_home_task", 8096, this, 5, &this->home_task_handle);
}