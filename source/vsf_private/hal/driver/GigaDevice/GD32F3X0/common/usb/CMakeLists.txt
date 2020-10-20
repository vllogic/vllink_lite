# CMakeLists head

target_sources(vsf INTERFACE
    usb_common.c
    usb_lv0.c
    usb.c
)

add_subdirectory(dc)
add_subdirectory(hc)
