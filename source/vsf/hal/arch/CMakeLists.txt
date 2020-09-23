# CMakeLists head

target_sources(vsf INTERFACE
    vsf_arch_abstraction.c
)

add_subdirectory(${VSFHAL_ARCH_SERIES})
