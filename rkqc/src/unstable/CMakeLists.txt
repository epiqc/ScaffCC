file( GLOB unstable_SRCS . optimization/*.cpp synthesis/*.cpp verification/*.cpp debugging/*.cpp simulation/*.cpp testing/*.cpp )
 
add_library( revkit_unstable ${unstable_SRCS} null.cpp )

file( GLOB INSTALL_HEADERS_OPTIMIZATION optimization/*.hpp )
file( GLOB INSTALL_HEADERS_SYNTHESIS synthesis/*.hpp )
file( GLOB INSTALL_HEADERS_VERIFICATION verification/*.hpp )
file( GLOB INSTALL_HEADERS_DEBUGGING debugging/*.hpp )
file( GLOB INSTALL_HEADERS_SIMULATION simulation/*.hpp )
file( GLOB INSTALL_HEADERS_TESTING testing/*.hpp )

install( FILES ${INSTALL_HEADERS_OPTIMIZATION} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/unstable/optimization )
install( FILES ${INSTALL_HEADERS_SYNTHESIS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/unstable/synthesis )
install( FILES ${INSTALL_HEADERS_VERIFICATION} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/unstable/verification)
install( FILES ${INSTALL_HEADERS_DEBUGGING} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/unstable/debugging)
install( FILES ${INSTALL_HEADERS_SIMULATION} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/unstable/simulation)
install( FILES ${INSTALL_HEADERS_TESTING} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/unstable/testing)
 
install( TARGETS revkit_unstable DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )

