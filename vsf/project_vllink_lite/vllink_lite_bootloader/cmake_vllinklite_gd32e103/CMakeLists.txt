# CMakeLists head

cmake_minimum_required(VERSION 3.13)

# How to buiild:
#   1. Build Debug
#     "cmake -GNinja -S. -Bbuild" or "cmake -GNinja -S. -Bbuild -DCMAKE_BUILD_TYPE=Debug"
#     cmake --build ./build
#     "rm -rf ./build" or "rm -r -fo ./build"
#   2. Build Release
#     "cmake -GNinja -S. -Bbuild -DCMAKE_BUILD_TYPE=Release"
#     cmake --build ./build
#     "rm -rf ./build" or "rm -r -fo ./build"

# How to config chip:
#   VSFHAL_CHIP_VENDOR -> chip vendor [arm, GigaDevice, Nuvoton, ST, TI, ...]
#   VSFHAL_CHIP_SERIES -> chip series [GD32VF103, STM32F7X0, AM335X, ...]
#   VSFHAL_CHIP_NAME -> chip full name [GD32VF103C8, STM32F730, AM3352, ...]
#   VSFHAL_ARCH_SERIES -> chip arch series [arm, mcs51, rv, x86, ...]
#   VSFHAL_ARCH_NAME -> chip arch name [cortex-m0, cortex-m0plus, cortex-m3, 
#     cortex-m4, cortex-m7, cortex-m23, cortex-m33, cortex-r4, ...]

# select chip and arch
set(VSFHAL_CHIP_VENDOR      GigaDevice)
set(VSFHAL_CHIP_SERIES      GD32E10X)
#set(VSFHAL_CHIP_NAME        GD32E103)
set(VSFHAL_ARCH_SERIES      arm)
set(VSFHAL_ARCH_NAME        cortex-m4)

set(PROJ_COMPILE_DEFINITIONS
  PROJ_CFG_VLLINKLITE
  BRD_CFG_VLLINKLITE_GD32E103
)

if(${VSFHAL_ARCH_SERIES} STREQUAL "arm")
  #set(ARM_FLOATING_HARDWARE     "")
  set(ARM_FLOATING_HARDWARE     -mfpu=fpv4-sp-d16)
  #set(ARM_FLOATING_ABI          soft)
  #set(ARM_FLOATING_ABI          softfp)
  set(ARM_FLOATING_ABI          hard)
endif()

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Debug")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# set VSF_BASE
set(VSF_BASE ${CMAKE_CURRENT_LIST_DIR}/../../.. CACHE PATH "VSF Base Directory")

# select toolchain
if(${VSFHAL_ARCH_SERIES} STREQUAL "arm")
  include(${VSF_BASE}/cmake/gnuarmemb.cmake)
else()
  message(FATAL_ERROR "Unknown ARCH: ${VSFHAL_ARCH_SERIES}")
endif()

# select toolchain
include(${VSF_BASE}/cmake/extensions.cmake)

get_filename_component(PROJ_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJ_NAME} C CXX ASM)

#set(CONFIG_SHELL_LINUX    1)
#message(STATUS "${PROJECT_SOURCE_DIR}")
#message(STATUS "${PROJECT_BINARY_DIR}")

set(SOURCE_FILES ../main.c)
#list(APPEND SOURCE_FILES ../vsf_dfu.c)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})

add_subdirectory(${VSF_BASE} ${CMAKE_CURRENT_BINARY_DIR}/vsf_base)

#add_definitions(-DDEBUG=1 -O0 -g3)
#add_definitions(-O3)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
  .  
  ..
  ../config
)

target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC
  ${PROJ_COMPILE_DEFINITIONS}
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
  vsf
)

file(WRITE compile_definitions.h "// Auto Generate\n\n")
foreach(def ${PROJ_COMPILE_DEFINITIONS})
  file(APPEND compile_definitions.h "#ifndef ${def}\n")
  file(APPEND compile_definitions.h "#   define ${def}\n")
  file(APPEND compile_definitions.h "#endif\n\n")
endforeach()

target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
  LINKER:-T${CMAKE_CURRENT_SOURCE_DIR}/gd32e103xx_boot.ld
  LINKER:-Map=output.map
)

#target_link_options(${CMAKE_PROJECT_NAME} PRIVATE
#
#)

set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES
  SUFFIX ".elf"
)

# Enable lto
set_property(TARGET ${CMAKE_PROJECT_NAME}
 PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O ihex ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.hex
  COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.bin
)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  USES_TERMINAL
)
