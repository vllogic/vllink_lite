# CMakeLists head

target_sources(vsf INTERFACE
    cortex_m_generic.c
)

add_subdirectory(SysTick)
