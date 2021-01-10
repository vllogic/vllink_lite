# CMakeLists head

target_sources(${VSF_LIB_NAME} INTERFACE
    driver.c
    jtag.c
    swd.c
    timestamp.c
)

add_subdirectory(common)
