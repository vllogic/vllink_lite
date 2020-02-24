# CMakeLists head

if(NOT IS_DIRECTORY ${VSF_BASE}/vsf_private/hal/driver/${VSFHAL_CHIP_VENDOR}/${VSFHAL_CHIP_SERIES})
  add_subdirectory_existed(${VSFHAL_CHIP_SERIES})
endif()
add_subdirectory(NuConsole)
