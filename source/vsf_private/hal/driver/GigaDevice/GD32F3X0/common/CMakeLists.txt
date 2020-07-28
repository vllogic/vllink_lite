# CMakeLists head

target_sources(vsfkernel INTERFACE
    __common.c
    adc.c
    dma.c
    flash.c
    i2c.c
    io.c
    pwm.c
    spi.c
    startup_gd32f3x0.c
    usart.c
    wdt.c
)

add_subdirectory(usb)
