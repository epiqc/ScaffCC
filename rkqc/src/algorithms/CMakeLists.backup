set( algorithms_SRCS
  optimization/adding_lines.cpp
  optimization/line_reduction.cpp
  optimization/lnn_optimization.cpp
  optimization/window_optimization.cpp

  simulation/partial_simulation.cpp
  simulation/sequential_simulation.cpp
  simulation/simple_simulation.cpp

  synthesis/dd_synthesis_p.cpp
  synthesis/bdd_synthesis.cpp
  synthesis/embed_truth_table.cpp
  synthesis/esop_synthesis.cpp
  synthesis/exact_synthesis.cpp
  synthesis/kfdd_synthesis.cpp
  synthesis/quantum_decomposition.cpp
  synthesis/reed_muller_synthesis.cpp
  synthesis/synthesis_utils_p.cpp
  synthesis/swop.cpp
  synthesis/transformation_based_synthesis.cpp
  synthesis/transposition_based_synthesis.cpp

  verification/equivalence_check.cpp
)

add_library( revkit_algorithms ${algorithms_SRCS} )

set( INSTALL_HEADERS_OPTIMIZATION
  optimization/optimization.hpp

  optimization/adding_lines.hpp
  optimization/line_reduction.hpp
  optimization/lnn_optimization.hpp
  optimization/window_optimization.hpp
)

set( INSTALL_HEADERS_SIMULATION
  simulation/simulation.hpp

  simulation/partial_simulation.hpp
  simulation/sequential_simulation.hpp
  simulation/simple_simulation.hpp
)

set( INSTALL_HEADERS_SYNTHESIS
  synthesis/synthesis.hpp

  synthesis/bdd_synthesis.hpp
  synthesis/embed_truth_table.hpp
  synthesis/esop_synthesis.hpp
  synthesis/exact_synthesis.hpp
  synthesis/kfdd_synthesis.hpp
  synthesis/quantum_decomposition.hpp
  synthesis/reed_muller_synthesis.hpp
  synthesis/swop.hpp
  synthesis/transformation_based_synthesis.hpp
  synthesis/transposition_based_synthesis.hpp
)

set ( INSTALL_HEADERS_VERIFICATION
    verification/verification.hpp

    verification/equivalence_check.hpp
    )


install( FILES ${INSTALL_HEADERS_OPTIMIZATION} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/algorithms/optmization )
install( FILES ${INSTALL_HEADERS_SIMULATION} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/algorithms/simulation )
install( FILES ${INSTALL_HEADERS_SYNTHESIS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/algorithms/synthesis )
install( FILES ${INSTALL_HEADERS_VERIFICATION} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/algorithms/verification )

install( TARGETS revkit_algorithms DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )
