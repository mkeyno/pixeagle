#ifndef OLED_96X16_HPP
#define OLED_96X16_HPP
	#include <drivers/device/i2c.h>
	#include <px4_platform_common/i2c_spi_buses.h>
	#include <string.h>
	#define OLED_I2C_ADDR       0x3C
	#define OLED_TOTAL_WIDTH    100
	#define OLED_VISIBLE_WIDTH  96
	#define OLED_HEIGHT         16
	#define OLED_PAGES          2
	#define DISPLAY_OFFSET      0
	#define OLED_BUFFER_SIZE    (OLED_TOTAL_WIDTH * OLED_PAGES)
	#define ICON_LEFT   0
	#define ICON_RIGHT  1
	
	
	class OLED_96x16 : public device::I2C
		{
		public:
			OLED_96x16(int bus, int bus_frequency, int address = OLED_I2C_ADDR);
			virtual ~OLED_96x16();
			virtual int init() override;
			void print_status();
			void clear();
			void update();
			void print(const char *text, uint8_t x, uint8_t y);
			void print_two_lines(const char *line1, const char *line2);
			void print_auto_wrap(const char *text);
			void print_large(const char *text);
			void draw_large_char(char c, uint8_t x);
			void draw_large_icon(uint8_t x, uint8_t icon_index);
			void draw_icon_large(const char *text, uint8_t icon_index, uint8_t direction);
			void draw_pixel(uint8_t x, uint8_t y);
			void clear_pixel(uint8_t x, uint8_t y);
		private:
			uint8_t _buffer[OLED_BUFFER_SIZE]{};
			bool _initialized{false};
			int send_command(uint8_t cmd);
			int send_data(const uint8_t *data, size_t len);
		};
#endif /* OLED_96X16_HPP */