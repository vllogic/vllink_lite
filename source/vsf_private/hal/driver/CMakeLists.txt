# CMakeLists head

if(IS_DIRECTORY ${VSFHAL_CHIP_VENDOR})
  add_subdirectory(${VSFHAL_CHIP_VENDOR})
endif()
