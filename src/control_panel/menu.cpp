#include "lcd1602.hpp"
#include "menu.hpp"
#include "freertos/FreeRTOS.h"
#include <format>
#include <iostream>

Menu::Menu(LCD1602 &lcd)
    : lcd(lcd),
      selected(false),
      hover(0),
      scroll(0)
{
}

void Menu::add_menu_item(MenuItem *item)
{
    this->items.push_back(item);
}

void Menu::joystick_event_handler(JosystickEvent event)
{
    cout << "event: " << event << endl;
    if (event == JosystickEvent::BUTTON)
    {
        this->selected = !this->selected;
        return;
    }

    if (this->selected)
    {
        this->items[this->hover]->josystick_handler(event);
        return;
    }

    switch (event)
    {
    case JosystickEvent::UP:
        if (this->hover != 0)
        {
            this->hover--;
        }
        break;
    case JosystickEvent::DOWN:
        if (this->hover + 1 >= this->items.size())
        {
            this->hover = this->items.size() - 1;
        }
        else
        {
            this->hover++;
        }
        break;
    default:
        break;
    }

    if (this->scroll > this->hover)
    {
        this->scroll = this->hover;
    }
    else if (this->scroll < this->hover)
    {
        this->scroll = this->hover - this->lcd.num_rows + 1;
    }
}

void Menu::task()
{
    while (true)
    {
        this->lcd.clear();
        for (int i = 0; i < this->lcd.num_rows; i++)
        {
            auto &item = items[scroll + i];

            char prefix = ' ';
            if (this->scroll + i == hover)
            {
                if (this->selected)
                    prefix = '*';
                else
                    prefix = '>';
            }

            this->lcd.put_cur(i, 0);
            this->lcd.send_char(prefix);
            this->lcd.put_cur(i, 1);
            auto row = item->get_display_row(this->lcd.num_columns - 1);
            this->lcd.send_string(row);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void Menu::start_task(Joystick &joystick)
{
    joystick.add_callback([this](JosystickEvent event)
                          { this->joystick_event_handler(event); });

    xTaskCreate([](void *param)
                { ((Menu *)param)->task(); }, "menu_task", 8096, this, 5, nullptr);
}
