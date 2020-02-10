# CMakeLists head

target_sources(vsfkernel INTERFACE
    busybox.c
)

add_subdirectory(shell)
