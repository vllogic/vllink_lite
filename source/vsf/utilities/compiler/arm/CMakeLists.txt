# CMakeLists head

target_sources(vsfkernel INTERFACE
    retarget_io.c
    signal.c
)

target_include_directories(vsfkernel INTERFACE
  CMSIS/Core/Include
  CMSIS/DSP/Include
  CMSIS/NN/Include
)
