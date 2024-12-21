#include "encoder.hpp"
#include <freertos/FreeRTOS.h>

void Encoder::task()
{
    bool last = false;
    while (true)
    {
        if (last == false && gpio_get_level(this->pin) == 0)
        {
            this->count++;
            last = true;
        }
        else if (last == true && gpio_get_level(this->pin) == 1)
        {
            last = false;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

Encoder::Encoder(gpio_num_t pin) : pin(pin), count(0)
{
}

Encoder::~Encoder()
{
    if (this->task_handle != nullptr)
        vTaskDelete(this->task_handle);
}

void Encoder::init()
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (uint64_t)(1 << this->pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);

    xTaskCreate([](void *param)
                { ((Encoder *)param)->task(); }, "encoder_task", 8096, this, 5, &this->task_handle);
}

int Encoder::read()
{
    return this->count;
}

void Encoder::reset()
{
    this->count = 0;
}
