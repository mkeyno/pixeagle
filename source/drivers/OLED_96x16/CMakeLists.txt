px4_add_module(
				MODULE drivers__oled__oled_96x16
				MAIN oled_96x16
				COMPILE_FLAGS
					-Os
				SRCS
					oled_96x16.cpp
					oled_96x16_driver.cpp
				DEPENDS
					platforms__common
					drivers_board
				)