#include "button.hpp"

Button::Button(gpio_num_t pin) : pin(pin), task_handle(nullptr), callbacks()
{
}

Button::~Button()
{
    if (this->task_handle != nullptr)
        vTaskDelete(this->task_handle);
}

void Button::init()
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (uint64_t)(1 << this->pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);

    xTaskCreate([](void *param)
                { ((Button *)param)->task(); }, "button_task", 1024, this, 5, &this->task_handle);
}

void Button::task()
{
    bool last = false;
    while (true)
    {
        if (last == false && gpio_get_level(this->pin) == 0)
        {
            for (const auto &callback : this->callbacks)
                callback();
            last = true;
        }
        else if (last == true && gpio_get_level(this->pin) == 1)
        {
            last = false;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void Button::add_callback(std::function<void()> callback)
{
    this->callbacks.push_back(callback);
}