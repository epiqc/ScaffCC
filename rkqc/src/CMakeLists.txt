project( revkit )

cmake_minimum_required( VERSION 2.6 )

set( CMAKE_CXX_FLAGS "-g -Wall -Wno-deprecated -fPIC -fno-strict-aliasing" )
set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -Werror" )
set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -ggdb" )

option( BUILD_BINDINGS "Build Python bindings" ON )
option( BUILD_UNSTABLE "Build unstable algorithms" OFF )
option( BUILD_EXAMPLES "Build examples" OFF )
option( BOOST_PATH "User Boost Path (e.g. by distribution), will override boost in libs" "")

if( BOOST_PATH )
  include_directories( ${BOOST_PATH}/include )
endif( BOOST_PATH )

include_directories( ${CMAKE_SOURCE_DIR}
                     ${CMAKE_SOURCE_DIR}/../libs/include
                     ${CMAKE_SOURCE_DIR}/libs/include)

if( BOOST_PATH )
  link_directories( ${BOOST_PATH}/lib )
endif( BOOST_PATH )

link_directories( ${CMAKE_SOURCE_DIR}/../libs/lib 
		  ${CMAKE_SOURCE_DIR}/libs/lib) 
		  
add_subdirectory( core )
add_subdirectory( algorithms )

if( BUILD_UNSTABLE )
  add_subdirectory( unstable )
endif( BUILD_UNSTABLE )

if( BUILD_BINDINGS )
  add_subdirectory( bindings )
endif( BUILD_BINDINGS )

if( BUILD_EXAMPLES )
  add_subdirectory( examples )
endif( BUILD_EXAMPLES )
