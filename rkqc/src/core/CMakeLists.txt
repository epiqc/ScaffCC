set( core_SRCS
  circuit.cpp
  gate.cpp
  pattern.cpp
  properties.cpp
  target_tags.cpp
  truth_table.cpp
  version.cpp

  io/create_image.cpp
  io/print_circuit.cpp
  io/print_statistics.cpp
  io/read_realization.cpp
  io/read_specification.cpp
  io/read_pattern.cpp
  io/read_pla.cpp
  io/revlib_parser.cpp
  io/revlib_processor.cpp
  io/write_blif.cpp
  io/write_realization.cpp
  io/write_specification.cpp
  io/write_verilog.cpp

  functions/active_controls.cpp
  functions/add_circuit.cpp
  functions/add_gates.cpp
  functions/add_line_to_circuit.cpp
  functions/circuit_hierarchy.cpp
  functions/circuit_to_truth_table.cpp
  functions/clear_circuit.cpp
  functions/copy_circuit.cpp
  functions/copy_metadata.cpp
  functions/create_simulation_pattern.cpp
  functions/expand_circuit.cpp
  functions/extend_truth_table.cpp
  functions/find_lines.cpp
  functions/flatten_circuit.cpp
  functions/fully_specified.cpp
  functions/reverse_circuit.cpp
  functions/transposition_to_circuit.cpp

  meta/bus_collection.cpp

  utils/costs.cpp
  utils/program_options.cpp
)

add_library( revkit_core ${core_SRCS} )

set( INSTALL_HEADERS_CORE
  gate.hpp
  circuit.hpp
  pattern.hpp
  properties.hpp
  target_tags.hpp
  truth_table.hpp
  version.hpp
)

set( INSTALL_HEADERS_IO
  io/create_image.hpp
  io/print_circuit.hpp
  io/print_statistics.hpp
  io/read_realization.hpp
  io/read_specification.hpp
  io/read_pattern.hpp
  io/read_pla.hpp
  io/revlib_parser.hpp
  io/revlib_processor.hpp
  io/write_blif.hpp
  io/write_realization.hpp
  io/write_specification.hpp
  io/write_verilog.hpp
)

set( INSTALL_HEADERS_FUNCTIONS
  functions/active_controls.hpp
  functions/add_circuit.hpp
  functions/add_gates.hpp
  functions/add_line_to_circuit.hpp
  functions/circuit_hierarchy.hpp
  functions/circuit_to_truth_table.hpp
  functions/clear_circuit.hpp
  functions/control_lines.hpp
  functions/copy_circuit.hpp
  functions/copy_metadata.hpp
  functions/expand_circuit.hpp
  functions/extend_truth_table.hpp
  functions/find_lines.hpp
  functions/flatten_circuit.hpp
  functions/fully_specified.hpp
  functions/reverse_circuit.hpp
  functions/target_lines.hpp
  functions/transposition_to_circuit.hpp
)

set( INSTALL_HEADERS_META
  meta/bus_collection.hpp
)

set( INSTALL_HEADERS_UTILS
  utils/costs.hpp
  utils/program_options.hpp
  utils/timer.hpp
)

install( FILES ${INSTALL_HEADERS_CORE} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/core )
install( FILES ${INSTALL_HEADERS_FUNCTIONS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/core/functions )
install( FILES ${INSTALL_HEADERS_IO} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/core/io )
install( FILES ${INSTALL_HEADERS_META} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/core/meta )
install( FILES ${INSTALL_HEADERS_UTILS} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/core/utils )

install( TARGETS revkit_core DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )
