#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "esp_log.h"
#include "lcd1602.hpp"

#define TAG "LCD1602"

LCD1602::LCD1602(uint8_t addr,
				 uint8_t num_rows,
				 uint8_t num_columns,
				 gpio_num_t master_scl_io,
				 gpio_num_t master_sda_io,
				 i2c_port_t i2c_port,
				 uint32_t master_freq_hz)
	: addr(addr),
	  num_rows(num_rows),
	  num_columns(num_columns),
	  master_scl_io(master_scl_io),
	  master_sda_io(master_sda_io),
	  i2c_port(i2c_port),
	  master_freq_hz(master_freq_hz),
	  bus_handle(nullptr),
	  lcd_handle(nullptr)
{
	if (this->bus_handle != nullptr || this->lcd_handle != nullptr) {
		ESP_LOGW(TAG, "LCD already initialized");
		return;
	}

	i2c_master_bus_config_t i2c_mst_config = {
		.i2c_port = this->i2c_port,
		.sda_io_num = this->master_sda_io,
		.scl_io_num = this->master_scl_io,
		.clk_source = I2C_CLK_SRC_DEFAULT,
		.glitch_ignore_cnt = 7,
		.flags = {
			.enable_internal_pullup = true
		}
	};

	ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &this->bus_handle));

	i2c_device_config_t dev_cfg = {
		.dev_addr_length = I2C_ADDR_BIT_LEN_7,
		.device_address = this->addr,
		.scl_speed_hz = this->master_freq_hz,
	};

	ESP_ERROR_CHECK(i2c_master_bus_add_device(this->bus_handle, &dev_cfg, &this->lcd_handle));

	// 4 bit initialisation
	vTaskDelay(pdMS_TO_TICKS(50)); // wait for >40ms
	this->send_cmd(0x30);
	esp_rom_delay_us(4500); // wait for >4.1ms
	this->send_cmd(0x30);
	esp_rom_delay_us(200); // wait for >100us
	this->send_cmd(0x30);
	esp_rom_delay_us(200);
	this->send_cmd(0x20); // 4bit mode
	esp_rom_delay_us(200);

	// dislay initialisation
	this->send_cmd(0x28); // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	esp_rom_delay_us(1000);
	this->send_cmd(0x08); // Display on/off control --> D=0,C=0, B=0  ---> display off
	esp_rom_delay_us(1000);
	this->send_cmd(0x01); // clear display
	esp_rom_delay_us(2000);
	this->send_cmd(0x06); // Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	esp_rom_delay_us(1000);
	this->send_cmd(0x0C); // Display on/off control --> D = 1, C and B = 0. (Cursor and blink, last two bits)
	esp_rom_delay_us(2000);
}

LCD1602::~LCD1602()
{
	if (this->lcd_handle != nullptr) {
		i2c_master_bus_rm_device(this->lcd_handle);
	}
	if (this->bus_handle != nullptr) {
		i2c_del_master_bus(this->bus_handle);
	}
}

void LCD1602::send_cmd(char cmd)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (cmd & 0xf0);
	data_l = ((cmd << 4) & 0xf0);
	data_t[0] = data_u | 0x0C; // en=1, rs=0
	data_t[1] = data_u | 0x08; // en=0, rs=0
	data_t[2] = data_l | 0x0C; // en=1, rs=0
	data_t[3] = data_l | 0x08; // en=0, rs=0
	ESP_ERROR_CHECK(i2c_master_transmit(this->lcd_handle, data_t, 4, 1000));
}

void LCD1602::put_cur(int row, int col)
{
	switch (row)
	{
	case 0:
		col |= 0x80;
		break;
	case 1:
		col |= 0xC0;
		break;
	}

	this->send_cmd(col);
}

void LCD1602::send_data(char data)
{
	char data_u, data_l;
	uint8_t data_t[4];
	data_u = (data & 0xf0);
	data_l = ((data << 4) & 0xf0);
	data_t[0] = data_u | 0x0D; // en=1, rs=1
	data_t[1] = data_u | 0x09; // en=0, rs=1
	data_t[2] = data_l | 0x0D; // en=1, rs=1
	data_t[3] = data_l | 0x09; // en=0, rs=1
	ESP_ERROR_CHECK(i2c_master_transmit(this->lcd_handle, data_t, 4, 1000));
}

void LCD1602::send_char(char ch)
{
	this->send_data(ch);
}

void LCD1602::send_string(std::string str)
{
	for (char c : str)
	{
		this->send_data(c);
	}
}

void LCD1602::clear()
{
	this->send_cmd(0x01);
	esp_rom_delay_us(2000);
}