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
 * @file expand_circuit.hpp
 *
 * @brief Expand a circuit on the base of a sub circuit
 */

#ifndef EXPAND_CIRCUIT_HPP
#define EXPAND_CIRCUIT_HPP

#include <core/circuit.hpp>

namespace revkit
{

  /**
   * @brief Expands a circuit
   *
   * This function expands the circuit \p base, such that
   * it will have \p num_lines and maps each line \em i
   * in the circuit \p base to the line \em filter[i] in the
   * circuit \p circ.
   *
   * @param base Base circuit
   * @param circ Newly created circuit, extended from \p base. Needs to be empty.
   * @param num_lines New number of lines
   * @param filter Mapping for calculating the new line indices.
   *
   * @return true on success, false otherwise
   *
   * @author RevKit
   * @since  1.0
   */
  bool expand_circuit( const circuit& base, circuit& circ, unsigned num_lines, const std::vector<unsigned>& filter );

  /**
   * @brief Expands a circuit
   *
   * As expand_circuit(const circuit&, circuit&, unsigned, const std::vector<unsigned>& but takes
   * the number of lines and the filter from the circuit::filter method.
   * This function requires \p base to be a subcircuit.
   *
   * @param base Base circuit. Needs to be a subcircuit.
   * @param circ Newly created circuit, extended from \p base. Needs to be empty.
   *
   * @return true on success, false otherwise
   *
   * @author RevKit
   * @since  1.0
   */
  bool expand_circuit( const circuit& base, circuit& circ );

}

#endif /* EXPAND_CIRCUIT_HPP */
