if("${PX4_BOARD_LABEL}" STREQUAL "bootloader")
    add_compile_definitions(BOOTLOADER)
    add_library(drivers_board
        bootloader_main.c
        init.c
        usb.c
        timer_config.cpp
    )
    target_link_libraries(drivers_board
        PRIVATE
            nuttx_arch
            px4_layer
            bootloader
    )
    target_include_directories(drivers_board
        PRIVATE
            ${PX4_SOURCE_DIR}/platforms/nuttx/src/bootloader/common
    )
else()
    add_library(drivers_board
        sched_stubs.c          # Add this
        boardctl_stub.c        # Add this  
        board_dma_alloc.c
        i2c.cpp
        init.c
        led.cpp
        mtd.cpp
        manifest.c
        sdio.c
        spi.cpp
        timer_config.cpp
        usb.c
    )
    
    target_link_libraries(drivers_board
        PRIVATE
            arch_spi
            arch_board_hw_info
            drivers__led
            nuttx_drivers
            nuttx_fs
            nuttx_mm
            nuttx_arch
            px4_layer
    )

    if(TARGET arch_spi)
        target_compile_options(arch_spi PRIVATE -Wno-unused-function)
    endif()
    
    # Allow PX4's cpuload to override NuttX's sched_note functions
    target_link_options(drivers_board INTERFACE -Wl,--allow-multiple-definition)
endif()

# target_compile_options(drivers_board PRIVATE -v)
# target_link_options(drivers_board PRIVATE -Wl,--verbose)