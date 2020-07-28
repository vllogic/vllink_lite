# CMakeLists head

target_sources(vsfkernel INTERFACE
    SEGGER_RTT.c
    SEGGER_RTT_ASM_ARMv7M.S
    SEGGER_RTT_printf.c
)
