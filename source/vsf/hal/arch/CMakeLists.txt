# CMakeLists head

target_sources(vsfkernel INTERFACE
  vsf_arch_abstraction.c
)

add_subdirectory_existed(${VSFHAL_ARCH_SERIES})
