/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2011  The RevKit Developers <revkit@informatik.uni-bremen.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file print_circuit.hpp
 *
 * @brief Console output of a circuit
 */

#ifndef PRINT_CIRCUIT_HPP
#define PRINT_CIRCUIT_HPP

#include <iostream>

#include <core/circuit.hpp>

namespace revkit
{

  class gate;

  /**
   * @brief Settings for print_circuit function
   *
   * @note This settings cannot be applied when
   *       using operator<<(std::ostream&, const circuit&)
   *       to print a circuit.
   *
   * @author RevKit
   * @since  1.0
   */
  struct print_circuit_settings
  {
    /**
     * @brief Default constructor
     *
     * Initializes default values
     *
     * @param os The stream parameter has to be determined in the constructor
     *
     * @author RevKit
     * @since  1.0
     */
    print_circuit_settings( std::ostream& os = std::cout );

    /**
     * @brief Default deconstructor
     *
     * @author RevKit
     * @since  1.1
     */
    virtual ~print_circuit_settings();

    /**
     * @brief The stream to dump the circuit to (default: \b std::ostream)
     *
     * @author RevKit
     * @since  1.0
     */
    std::ostream& os;

    /**
     * @brief Determines whether the inputs and outputs should be printed (default: \b false)
     *
     * @author RevKit
     * @since  1.0
     */
    bool print_inputs_and_outputs;

    /**
     * @brief Determines whether the gate_index should be printed (default: \b false)
     *
     * If this flag is enabled, in the first line --
     * before the circuit is printed -- the gate
     * index is printed as modulo to base 10, because
     * there is only space for one character.
     *
     * @note The first gate has the index 0.
     *
     * @author RevKit
     * @since  1.0
     */
    bool print_gate_index;

    /**
     * @brief Character to be printed for a control line
     *
     * Default value is \b *
     *
     * @author RevKit
     * @since 1.0
     */
    char control_char;
    
    /**
     * @brief Character to be printed for an empty line
     *
     * Default value is \b -
     *
     * @author RevKit
     * @since 1.0
     */
    char line_char;

    /**
     * @brief Space between gates
     *
     * Default value is \b 0.
     *
     * @author RevKit
     * @since 1.0
     */
    unsigned gate_spacing;

    /**
     * @brief Space between lines
     *
     * Default value is \b 0.
     *
     * @author RevKit
     * @since  1.0
     */
    unsigned line_spacing;

    /**
     * @brief Returns a char for a gate
     *
     * The default implementation returns \b O for Toffoli
     * and Peres gates, \b X for Fredkin, \b v for V gates,
     * and \b + for V+ gates.
     *
     * For unknown gates an empty char ' ' is returned.
     *
     * When overriding this method, first the base method can be called
     * and further decisions can be made when the return value is an
     * empty char meaning it is an unknown type.
     *
     * @param g Gate
     *
     * @return The char
     */
    virtual char target_type_char( const gate& g ) const;
  };

  /**
   * @brief Prints a circuit as ASCII
   *
   * This method can be used to dump a circuit
   * as ASCII to the console output or into 
   * debug files.
   *
   * @param circ     Circuit
   * @param settings Settings (see print_circuit_settings)
   *
   * @author RevKit
   * @since  1.0
   */
  void print_circuit( const circuit& circ, const print_circuit_settings& settings = print_circuit_settings() );

  /**
   * @brief Wrapper for using with the output stream operator
   *
   * This operator wraps the print_circuit method to output a circuit
   * in a stream flow using the left shift operator.
   *
   * @param os   The stream to dump the circuit
   * @param circ Circuit
   *
   * @return The stream given as parameter \p os
   *
   * @author RevKit
   * @since  1.0
   */
  std::ostream& operator<<( std::ostream& os, const circuit& circ );

}

#endif /* PRINT_CIRCUIT_HPP */
