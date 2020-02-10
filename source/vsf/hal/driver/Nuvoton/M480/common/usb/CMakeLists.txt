# CMakeLists head

target_sources(vsfkernel INTERFACE
    usb_lv0.c
    usb.c
)

add_subdirectory(dc)
add_subdirectory(ohci)
