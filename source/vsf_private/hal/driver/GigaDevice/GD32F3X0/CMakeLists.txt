# CMakeLists head

target_sources(vsf INTERFACE
    driver.c
    jtag.c
    swd.c
    timestamp.c
)

add_subdirectory(common)
