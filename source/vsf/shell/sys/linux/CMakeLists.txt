# CMakeLists head

target_include_directories(vsfkernel INTERFACE
  ./include
)

target_sources(vsfkernel INTERFACE
    vsf_linux_socket.c
    vsf_linux.c
)

add_subdirectory(lib)
add_subdirectory(port)
