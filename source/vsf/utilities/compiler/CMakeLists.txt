# CMakeLists head

add_subdirectory(__common)
if(IS_DIRECTORY ${VSFHAL_ARCH_SERIES})
  add_subdirectory(${VSFHAL_ARCH_SERIES})
endif()
