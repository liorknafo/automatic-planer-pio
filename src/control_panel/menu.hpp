#pragma once
#include "lcd1602.hpp"
#include "joystick.hpp"
#include <vector>
#include <string>
#include <memory>
#include "menu_items/menu_item.hpp"
using namespace std;

class Menu
{
private:
    LCD1602 &lcd;
    vector<unique_ptr<MenuItem>> items;
    TaskHandle_t task_handle;

    bool selected;
    size_t hover;
    size_t scroll;

    void task();
    void joystick_event_handler(JosystickEvent event);

public:
    Menu(LCD1602 &lcd);
    ~Menu();

    void add_menu_item(unique_ptr<MenuItem> item);

    void start_task(Joystick &joystick);
};