# CMakeLists head

add_subdirectory(common)
if(IS_DIRECTORY ${VSFHAL_CHIP_NAME})
  add_subdirectory(${VSFHAL_CHIP_NAME})
endif()