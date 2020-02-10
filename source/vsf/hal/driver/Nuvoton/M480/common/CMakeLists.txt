# CMakeLists head

target_sources(vsfkernel INTERFACE
    __common.c
)

add_subdirectory(io)
add_subdirectory(pm)
add_subdirectory(usart)
add_subdirectory(usb)
add_subdirectory(vendor)
