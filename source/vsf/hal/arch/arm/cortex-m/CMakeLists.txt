# CMakeLists head

target_sources(vsfkernel INTERFACE
    cortex_m_generic.c
)

add_subdirectory(SysTick)
