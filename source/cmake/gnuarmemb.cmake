# CMakeLists head

set(CMAKE_SYSTEM_NAME           Generic)
set(CMAKE_SYSTEM_PROCESSOR      ${VSFHAL_ARCH_SERIES})

set(CMAKE_C_COMPILER arm-none-eabi-gcc CACHE INTERNAL "c compiler")
set(CMAKE_CXX_COMPILER arm-none-eabi-g++ CACHE INTERNAL "cxx compiler")
set(CMAKE_ASM_COMPILER arm-none-eabi-gcc CACHE INTERNAL "asm compiler")

set(CMAKE_OBJCOPY arm-none-eabi-objcopy CACHE INTERNAL "objcopy")
set(CMAKE_OBJDUMP arm-none-eabi-objdump CACHE INTERNAL "objdump")
set(CMAKE_SIZE arm-none-eabi-size CACHE INTERNAL "size")

set(CMAKE_C_FLAGS 
  "-mthumb -mcpu=${VSFHAL_ARCH_NAME} -mfloat-abi=${ARM_FLOATING_ABI} ${ARM_FLOATING_HARDWARE} \
    -std=gnu11 -Wall -fno-builtin -fno-strict-aliasing -fdata-sections -fms-extensions -ffunction-sections"
  CACHE INTERNAL "c compiler flags")
set(CMAKE_CXX_FLAGS 
  "-mthumb -mcpu=${VSFHAL_ARCH_NAME} -mfloat-abi=${ARM_FLOATING_ABI} ${ARM_FLOATING_HARDWARE} \
    -Wall -fno-builtin -fno-strict-aliasing -fdata-sections -fms-extensions -ffunction-sections"
  CACHE INTERNAL "cxx compiler flags")
set(CMAKE_ASM_FLAGS "-mthumb -mcpu=${VSFHAL_ARCH_NAME}" CACHE INTERNAL "asm compiler flags")

# Optimization Level
#   -O0       None
#   -Os       Optimize for size
#   -O1       Optimize
#   -O2       Optimize more
#   -O3       Optimize most
#   -Og       Optimize for debug
#   -Ofast    Optimize for speed
set(CMAKE_C_FLAGS_DEBUG "-Og -g3 -gstabs+" CACHE INTERNAL "c debug compiler flags")
set(CMAKE_CXX_FLAGS_DEBUG "-Og -g3 -gstabs+" CACHE INTERNAL "cxx debug compiler flags")
set(CMAKE_ASM_FLAGS_DEBUG "-g -gstabs+" CACHE INTERNAL "asm debug compiler flags")

set(CMAKE_C_FLAGS_RELEASE "-Os" CACHE INTERNAL "c release compiler flags")
set(CMAKE_CXX_FLAGS_RELEASE "-Os" CACHE INTERNAL "cxx release compiler flags")
set(CMAKE_ASM_FLAGS_RELEASE "" CACHE INTERNAL "asm release compiler flags")

set(CMAKE_EXE_LINKER_FLAGS "-Wl,--gc-sections --specs=nosys.specs"
  CACHE INTERNAL "exe link flags")

set(CMAKE_C_COMPILER_WORKS ON)
set(CMAKE_CXX_COMPILER_WORKS ON)
