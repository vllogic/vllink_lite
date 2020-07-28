# CMakeLists head

target_sources(vsfkernel INTERFACE
    vsf_tgui.c
    vsf_tgui_common.c
)

add_subdirectory(controls)
add_subdirectory(utilities)
add_subdirectory(view)
