# CMakeLists head

target_sources(vsfkernel INTERFACE
    __common.c
    dma.c
    flash.c
    io.c
    spi.c
    startup_gd32e10x.c
    usart.c
)

add_subdirectory(usb)
