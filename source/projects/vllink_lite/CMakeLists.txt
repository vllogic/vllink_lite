# CMakeLists head

# How to buiild:
#   1. build in bin directory
#       mkdir build
#       cd build
#       cmake -GNinja -S.. -B.
#       ninja
#       -> ninja clean
#   2. build in source directory
#       cmake -GNinja -S. -Bbuild
#       cmake --build ./build
#       ->rm -rf ./build or rm -r -fo ./build

cmake_minimum_required(VERSION 3.13)

# set VSF_BASE
set(VSF_BASE ${CMAKE_CURRENT_LIST_DIR}/../.. CACHE PATH "VSF Base Directory")

# select chip and arch
set(VSFHAL_CHIP_VENDOR      GigaDevice)
set(VSFHAL_CHIP_SERIES      GD32F3X0)
set(VSFHAL_CHIP_NAME        GD32F350)
set(VSFHAL_ARCH_SERIES      arm)
set(VSFHAL_ARCH_NAME        cortex-m4)

# select toolchain
include(${VSF_BASE}/cmake/gnuarmemb.cmake)

# select toolchain
include(${VSF_BASE}/cmake/extensions.cmake)

project(kernel_test C CXX ASM)

#set(CONFIG_SHELL_LINUX    1)
#message(STATUS "${PROJECT_SOURCE_DIR}")
#message(STATUS "${PROJECT_BINARY_DIR}")

set(SOURCE_FILES main.c)
list(APPEND SOURCE_FILES usb_device.c)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})

add_subdirectory(${VSF_BASE} ${CMAKE_CURRENT_BINARY_DIR}/vsf_base)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
  .
  config
)

target_compile_definitions(${CMAKE_PROJECT_NAME}
  PUBLIC
    __GigaDevice__
    __GD32F350__
)

target_link_libraries(${CMAKE_PROJECT_NAME}
  PRIVATE
    vsfkernel
)

target_link_options(${CMAKE_PROJECT_NAME}
  PRIVATE
    LINKER:-Map=output.map
)

target_link_options(${CMAKE_PROJECT_NAME}
  PRIVATE
    --specs=nosys.specs
)

set_target_properties(${CMAKE_PROJECT_NAME}
  PROPERTIES
    SUFFIX ".elf"
)

add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    USES_TERMINAL
)
