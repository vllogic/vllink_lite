# CMakeLists head

target_sources(vsfkernel INTERFACE
  vsf_arch_abstraction.c
)

if(IS_DIRECTORY ${VSFHAL_ARCH_SERIES})
  add_subdirectory(${VSFHAL_ARCH_SERIES})
endif()
