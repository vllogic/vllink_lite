# CMakeLists head

target_sources(vsfkernel INTERFACE
    cstartup.s51
    mcs51_generic.c
    POST_STARTUP.A51
)

#vsf_compile_options()

#vsf_link_options()
