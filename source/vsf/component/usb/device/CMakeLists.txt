# CMakeLists head

target_sources(vsfkernel INTERFACE
    vsf_usbd.c
)

add_subdirectory(class)
