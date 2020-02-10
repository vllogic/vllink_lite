# CMakeLists head

target_sources(vsfkernel INTERFACE
  vsf_hal.c
)

add_subdirectory(arch)
add_subdirectory(driver)
add_subdirectory(interface)
