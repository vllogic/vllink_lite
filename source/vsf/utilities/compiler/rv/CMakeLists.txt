# CMakeLists head

target_sources(vsfkernel INTERFACE
    retarget_io.c
    signal.c
)

add_subdirectory(RISCV)
