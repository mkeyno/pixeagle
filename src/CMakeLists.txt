if("${PX4_BOARD_LABEL}" STREQUAL  "bootloader")
	add_library(drivers_board
		bootloader_main.c
		usb.c
		oled_debug.c
	)
	target_link_libraries(drivers_board
		PRIVATE
			nuttx_arch
			nuttx_drivers
			bootloader
			arch_bootloader
			arch_hrt
	)
	target_include_directories(drivers_board PRIVATE ${PX4_SOURCE_DIR}/platforms/nuttx/src/bootloader/common)
else()
	add_library(drivers_board
		can.c
		i2c.cpp
		init.c
		manifest.c
		spi.cpp
		spi_register.c       # CS callback handlers
		timer_config.cpp
		usb.c
		oled_debug.c
		board_hw_info.c
		board_mtd.cpp          # MTD manifest (was mtd.cpp)
		board_ioctl.c         # Board-specific IOCTL functions
		bch.c                 # BCH driver for SD card (added back)
	)
	add_dependencies(drivers_board arch_board_hw_info)
	target_link_libraries(drivers_board
		PRIVATE
			arch_io_pins
			arch_spi
			arch_board_hw_info
			drivers__led
			nuttx_arch
			nuttx_drivers
			px4_layer
	)
endif()
