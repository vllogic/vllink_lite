# CMakeLists head

target_sources(vsfkernel INTERFACE
    vsf_fs.c
)

add_subdirectory(driver)
