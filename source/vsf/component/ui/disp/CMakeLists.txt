# CMakeLists head

target_sources(vsfkernel INTERFACE
    vsf_disp.c
)

add_subdirectory(driver)
