# CMakeLists head

target_sources(vsfkernel INTERFACE
    segger_rtt_stream.c
)

add_subdirectory(RTT)
