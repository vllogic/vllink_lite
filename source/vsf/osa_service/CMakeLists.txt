# CMakeLists head

target_sources(vsfkernel INTERFACE
    vsf_osa_service.c
)

add_subdirectory(msg_tree)
