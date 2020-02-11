# CMakeLists head

if(IS_DIRECTORY ${VSFHAL_CHIP_SERIES})
  add_subdirectory(${VSFHAL_CHIP_SERIES})
endif()