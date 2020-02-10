# CMakeLists head

target_sources(vsfkernel INTERFACE
    vsf_linux_glibc_pthread.c
    vsf_linux_glibc_sem.c
    vsf_linux_glibc_stdio.c
    vsf_linux_glibc_stdlib.c
    vsf_linux_glibc_string.c
    vsf_linux_glibc_time.c
    vsf_linux_glibc.c
)
