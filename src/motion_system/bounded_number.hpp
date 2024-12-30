#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include <algorithm>

template <typename T>
class BoundedNumber {
private:
    T value;
    T min;
    T max;
    SemaphoreHandle_t mutex;

public:
    BoundedNumber(T initial, T min, T max) : value(initial), min(min), max(max) {
        this->mutex = xSemaphoreCreateMutex();
    }

    ~BoundedNumber() {
        if (this->mutex != nullptr) {
            vSemaphoreDelete(this->mutex);
        }
    }

    T get() {
        xSemaphoreTake(this->mutex, portMAX_DELAY);
        T val = this->value;
        xSemaphoreGive(this->mutex);
        return val;
    }

    void set(T val) {
        xSemaphoreTake(this->mutex, portMAX_DELAY);
        this->value = std::max(this->min, std::min(this->max, val));
        xSemaphoreGive(this->mutex);
    }

    void increment(T amount) {
        xSemaphoreTake(this->mutex, portMAX_DELAY);
        this->value = std::min(this->max, this->value + amount);
        xSemaphoreGive(this->mutex);
    }

    void decrement(T amount) {
        xSemaphoreTake(this->mutex, portMAX_DELAY);
        this->value = std::max(this->min, this->value - amount);
        xSemaphoreGive(this->mutex);
    }
}; 