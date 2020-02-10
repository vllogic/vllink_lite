# CMakeLists head

target_sources(vsfkernel INTERFACE
    vsf_mal.c
)

add_subdirectory(driver)
