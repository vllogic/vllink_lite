# CMakeLists head

target_sources(vsf INTERFACE
    retarget_io.c
    signal.c
)

target_include_directories(vsf INTERFACE
  CMSIS/Core/Include
  CMSIS/DSP/Include
  CMSIS/NN/Include
)
