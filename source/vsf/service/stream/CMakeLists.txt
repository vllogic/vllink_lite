# CMakeLists head

target_sources(vsfkernel INTERFACE
    vsf_stream_base.c
    vsf_stream_reader.c
    vsf_stream_writer.c
    vsf_stream.c
)

add_subdirectory(pbuf_queue)
