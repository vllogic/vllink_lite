# CMakeLists head

target_sources(vsfkernel INTERFACE
    flash_lv0.c
    io_lv0.c
    usart_lv0.c
    startup_GD32F3X0.c
    __common.c
)

add_subdirectory(flash)
add_subdirectory(io)
add_subdirectory(startup)
add_subdirectory(usart)
add_subdirectory(usb)
