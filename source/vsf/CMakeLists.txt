# CMakeLists head

target_include_directories(vsfkernel INTERFACE
  .
)

add_subdirectory(component)
add_subdirectory(hal)
add_subdirectory(kernel)
add_subdirectory(osa_hal)
add_subdirectory(osa_service)
add_subdirectory(service)
add_subdirectory(shell)
add_subdirectory(utilities)
