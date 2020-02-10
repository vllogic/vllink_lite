# CMakeLists head

target_sources(vsfkernel INTERFACE
    vsf_disp_drv.c
)

add_subdirectory(sdl2)
add_subdirectory(usbd_uvc)
