# CMakeLists head

target_sources(vsfkernel INTERFACE
    vsf_scsi.c
)

add_subdirectory(driver)
