if("${PX4_BOARD_LABEL}" STREQUAL  "bootloader")
	add_library(drivers_board
		bootloader_main.c
		usb.c
	)
	target_link_libraries(drivers_board
		PRIVATE
			nuttx_arch # sdio
			nuttx_drivers # sdio
			bootloader
	)
	target_include_directories(drivers_board PRIVATE ${PX4_SOURCE_DIR}/platforms/nuttx/src/bootloader/common)
else()
	add_library(drivers_board
		can.c
		i2c.cpp
		init.c
		led.c
		manifest.c
		sdio.c
		spi.cpp
		spi_register.c
		timer_config.cpp
		usb.c
	)
	add_dependencies(drivers_board arch_board_hw_info)
	target_link_libraries(drivers_board
		PRIVATE
			arch_io_pins
			arch_spi
			arch_board_hw_info
			drivers__led # drv_led_start
			nuttx_arch # sdio
			nuttx_drivers # sdio
			px4_layer
	)
endif()