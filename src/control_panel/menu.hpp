#pragma once
#include "lcd1602.hpp"
#include "joystick.hpp"
#include <vector>
#include <string>
#include "menu_items/menu_item.hpp"
using namespace std;

class Menu
{
private:
    LCD1602 &lcd;
    vector<MenuItem*> items;

    bool selected;

    uint32_t hover;
    uint32_t scroll;

    void task();
    void joystick_event_handler(JosystickEvent event);

public:
    Menu(LCD1602 &lcd);

    void add_menu_item(MenuItem* item);

    void start_task(Joystick &joystick);
};