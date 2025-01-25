#include "lcd1602.hpp"
#include "menu.hpp"
#include "freertos/FreeRTOS.h"
#include <format>
#include "esp_log.h"

static const char* TAG = "Menu";

Menu::Menu(LCD1602 &lcd)
    : lcd(lcd),
      task_handle(nullptr),
      selected(false),
      hover(0),
      scroll(0)
{
}

Menu::~Menu()
{
    if (this->task_handle != nullptr) {
        vTaskDelete(this->task_handle);
    }
}

void Menu::add_menu_item(unique_ptr<MenuItem> item)
{
    this->items.push_back(std::move(item));
}

void Menu::joystick_event_handler(JosystickEvent event)
{
    if (this->items.empty()) {
        return;
    }

    ESP_LOGD(TAG, "event: %d", static_cast<int>(event));
    
    if (event == JosystickEvent::BUTTON)
    {
        this->selected = !this->selected;
        if(this->selected)
            this->items[this->hover]->on_select();
        else
            this->items[this->hover]->on_unselect();
        return;
    }

    if (this->selected && this->hover < this->items.size())
    {
        this->items[this->hover]->josystick_handler(event);
        return;
    }

    switch (event)
    {
    case JosystickEvent::UP:
        if (this->hover > 0)
        {
            this->hover--;
        }
        break;
    case JosystickEvent::DOWN:
        if (this->hover + 1 < this->items.size())
        {
            this->hover++;
        }
        break;
    default:
        break;
    }

    // Update scroll position to keep hover in view
    if (this->scroll > this->hover)
    {
        this->scroll = this->hover;
    }
    else if (this->scroll + this->lcd.num_rows <= this->hover)
    {
        this->scroll = this->hover - this->lcd.num_rows + 1;
    }

    // Ensure scroll doesn't exceed maximum possible scroll position
    size_t max_scroll = (this->items.size() > this->lcd.num_rows) ? 
                       this->items.size() - this->lcd.num_rows : 
                       0;
    if (this->scroll > max_scroll)
    {
        this->scroll = max_scroll;
    }
}

void Menu::task()
{
    while (true)
    {
        this->lcd.clear();
        if (this->items.empty()) {
            vTaskDelay(pdMS_TO_TICKS(200));
            continue;
        }

        for (size_t i = 0; i < static_cast<size_t>(this->lcd.num_rows) && (this->scroll + i) < this->items.size(); i++)
        {
            auto &item = this->items[this->scroll + i];
            char prefix = ' ';
            if (this->scroll + i == this->hover)
            {
                if (this->selected)
                    prefix = '*';
                else
                    prefix = '>';
            }

            this->lcd.put_cur(static_cast<int>(i), 0);
            this->lcd.send_char(prefix);
            this->lcd.put_cur(static_cast<int>(i), 1);
            auto row = item->get_display_row(this->lcd.num_columns - 1);
            this->lcd.send_string(row);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void Menu::start_task(Joystick &joystick)
{
    if (this->task_handle != nullptr) {
        return;  // Task already running
    }

    joystick.add_callback([this](JosystickEvent event)
                          { this->joystick_event_handler(event); });

    BaseType_t result = xTaskCreate([](void *param)
                { ((Menu *)param)->task(); }, "menu_task", 8096, this, 5, &this->task_handle);

    if (result != pdPASS || this->task_handle == nullptr) {
        ESP_LOGE(TAG, "Failed to create menu task");
    }
}
