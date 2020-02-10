# CMakeLists head

target_sources(vsfkernel INTERFACE
    vsfstream.c
)

add_subdirectory(fifo_stream)
add_subdirectory(mem_stream)
