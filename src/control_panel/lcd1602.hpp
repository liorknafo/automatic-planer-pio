#pragma once
#include "driver/i2c_master.h"
#include <string>

class LCD1602
{
private:
    uint8_t addr;

public:
    uint8_t num_rows;
    uint8_t num_columns;

private:
    gpio_num_t master_scl_io;
    gpio_num_t master_sda_io;
    i2c_port_t i2c_port;
    uint32_t master_freq_hz;

    i2c_master_bus_handle_t bus_handle = nullptr;
    i2c_master_dev_handle_t lcd_handle = nullptr;

    void init_master();
    void send_cmd(char cmd);
    void send_data(char data);

public:
    LCD1602(uint8_t addr,
            uint8_t num_rows,
            uint8_t num_columns,
            gpio_num_t master_scl_io,
            gpio_num_t master_sda_io,
            i2c_port_t master_num,
            uint32_t master_freq_hz);
    ~LCD1602();

    void init();
    void put_cur(int row, int col);
    void send_string(std::string str);
    void send_char(char str);
    void clear();
};