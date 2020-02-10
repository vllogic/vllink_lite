# CMakeLists head

target_sources(vsfkernel INTERFACE
    vsf_input.c
)

add_subdirectory(driver)
add_subdirectory(protocol)
