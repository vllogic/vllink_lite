# CMakeLists head

target_sources(vsfkernel INTERFACE
    __common.c
)

add_subdirectory(flash)
add_subdirectory(io)
add_subdirectory(startup)
add_subdirectory(usart)
add_subdirectory(usb)
