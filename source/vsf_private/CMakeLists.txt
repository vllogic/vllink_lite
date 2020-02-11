# CMakeLists head

target_include_directories(vsfkernel INTERFACE
  .
)

add_subdirectory(component)
add_subdirectory(hal)
