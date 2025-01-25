#include "motion_system.hpp"
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include <iostream>

static const char* TAG = "MotionSystem";

void MotionSystem::move_x_to_end()
{
    this->motorX.move_forward(this->cutting_speed->get());
    this->xEndLimit.wait_for(true);
    this->motorX.stop();
}

void MotionSystem::move_x_to_start()
{
    this->motorX.move_backward(this->cutting_speed->get());
    this->xStartLimit.wait_for(true);
    this->motorX.stop();
}

bool MotionSystem::move_y_steps()
{
    this->yEncoder.reset();
    this->motorY.move_forward(this->cutting_speed->get());
    while (true)
    {
        if (this->yEndLimit.get())
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
    ESP_LOGI(TAG, "Moving");
    while (true)
    {
        ESP_LOGI(TAG, "Moving X to end");
        this->move_x_to_end();
        ESP_LOGI(TAG, "Moving Y steps");
        if (this->move_y_steps())
            break;
        ESP_LOGI(TAG, "Moving X to start");
        this->move_x_to_start();
        ESP_LOGI(TAG, "Moving Y steps");
        if (this->move_y_steps())
            break;
    }
    ESP_LOGI(TAG, "Moving done");
    this->move_task_handle = nullptr;
    vTaskDelete(nullptr);
}

void MotionSystem::home_task()
{
    ESP_LOGI(TAG, "Homing");
    float speed = this->travel_speed->get();
    this->motorX.move_backward(speed);
    this->motorY.move_backward(speed);
    while (1)
    {
        bool xDone = this->xStartLimit.get();
        if (xDone)
            this->motorX.stop();
        bool yDone = this->yStartLimit.get();
        if (yDone)
            this->motorY.stop();
        if (xDone && yDone)
            break;
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    ESP_LOGI(TAG, "Homing done");
    this->home_task_handle = nullptr;
    vTaskDelete(nullptr);
}

void MotionSystem::move_by_joystick_task()
{
    ESP_LOGI(TAG, "Moving by Joystick");
    while (true)
    {
        auto pos = this->joystick.get_position();
        if (!pos.has_value())
        {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        auto [x, y] = *pos;

        // Handle X motor movement with limits
        if ((x > 0 && !this->xEndLimit.get()) || 
            (x < 0 && !this->xStartLimit.get())) {
            this->motorX.move(x);
        } else {
            this->motorX.stop();
        }

        // Handle Y motor movement with limits
        if ((y > 0 && !this->yEndLimit.get()) || 
            (y < 0 && !this->yStartLimit.get())) {
            this->motorY.move(y);
        } else {
            this->motorY.stop();
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
    ESP_LOGI(TAG, "Joystick moving done");
    this->move_by_joystick_task_handle = nullptr;
    vTaskDelete(nullptr);
}

void MotionSystem::move_by_joystick()
{
    if (this->move_task_handle != nullptr || this->home_task_handle != nullptr || this->move_by_joystick_task_handle != nullptr)
        return;
    xTaskCreate([](void *param)
                { ((MotionSystem *)param)->move_by_joystick_task(); },
                "motion_system_move_by_joystick_task",
                16384,
                this,
                5,
                &this->move_by_joystick_task_handle);
}

void MotionSystem::stop_move_by_joystick()
{
    if (this->move_by_joystick_task_handle != nullptr)
    {
        TaskHandle_t joystick_handle = this->move_by_joystick_task_handle;
        this->move_by_joystick_task_handle = nullptr;
        vTaskDelete(joystick_handle);
    }
    this->motorY.stop();
    this->motorX.stop();
}

MotionSystem::MotionSystem(Motor &motorX, Motor &motorY,
                           gpio_num_t yEncoder, gpio_num_t yStartLimit, gpio_num_t yEndLimit,
                           gpio_num_t xStartLimit, gpio_num_t xEndLimit,
                           std::shared_ptr<BoundedNumber<float>> cutting_speed,
                           std::shared_ptr<BoundedNumber<float>> travel_speed,
                           std::shared_ptr<BoundedNumber<uint32_t>> y_offset,
                           Joystick &joystick)
    : motorX(motorX),
      motorY(motorY),
      yEncoder(yEncoder),
      yStartLimit(yStartLimit),
      yEndLimit(yEndLimit),
      xStartLimit(xStartLimit),
      xEndLimit(xEndLimit),
      move_task_handle(nullptr),
      home_task_handle(nullptr),
      move_by_joystick_task_handle(nullptr), // Initialize move_by_joystick_task_handle
      cutting_speed(cutting_speed),
      travel_speed(travel_speed),
      y_offset(y_offset),
      joystick(joystick) // Initialize joystick
{
}

MotionSystem::~MotionSystem()
{
    stop();                  // Ensure all tasks are stopped
    stop_move_by_joystick(); // Ensure move_by_joystick task is stopped
}

void MotionSystem::move()
{
    if (this->move_task_handle != nullptr || this->home_task_handle != nullptr || this->move_by_joystick_task_handle != nullptr)
        return;
    xTaskCreate([](void *param)
                { ((MotionSystem *)param)->move_task(); },
                "motion_system_move_task",
                16384,
                this,
                5,
                &this->move_task_handle);
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
    if (this->move_task_handle != nullptr || this->home_task_handle != nullptr || this->move_by_joystick_task_handle != nullptr)
        return;
    xTaskCreate([](void *param)
                { ((MotionSystem *)param)->home_task(); }, "motion_system_home_task", 8096, this, 5, &this->home_task_handle);
}