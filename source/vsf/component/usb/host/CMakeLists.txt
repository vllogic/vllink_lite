# CMakeLists head

target_sources(vsfkernel INTERFACE
    vsf_usbh.c
)

add_subdirectory(class)
