# CMakeLists head

function(add_subdirectory_ifdef feature_toggle dir)
  if(${${feature_toggle}})
    add_subdirectory(${dir})
  endif()
endfunction()

function(add_subdirectory_existed dir)
  get_filename_component(_fullpath "${dir}" REALPATH)
  if(IS_DIRECTORY ${_fullpath})
    add_subdirectory(${_fullpath})
  endif()
endfunction()
