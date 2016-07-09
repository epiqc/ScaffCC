find_package( PythonLibs REQUIRED )

include_directories( ${PYTHON_INCLUDE_PATH} )

set( SRCS
  python.cpp

  core.cpp
  core_functions.cpp
  core_io.cpp
  core_utils.cpp

  algorithms.cpp
  unstable.cpp

  vector_py.cpp
)

add_library( revkit_python SHARED ${SRCS} )
set_target_properties( revkit_python PROPERTIES PREFIX "" )
target_link_libraries(
  revkit_python

  revkit_unstable
  revkit_algorithms
  revkit_core

  boost_chrono
  boost_python
  boost_system
  boost_filesystem
  boost_regex
  boost_program_options
  boost_thread
  boost_signals

  _puma

  CUDD_cudd
  CUDD_dddmp
  CUDD_epd
  CUDD_mtr
  CUDD_st
  CUDD_util

  fmi
  minisat-fmi

)

install( TARGETS revkit_python DESTINATION ${CMAKE_INSTALL_PREFIX}/bindings )
