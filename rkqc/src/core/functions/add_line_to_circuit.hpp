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
 * @file add_line_to_circuit.hpp
 *
 * @brief Add a line to a circuit with specifying all meta-data
 */

#ifndef ADD_LINE_TO_CIRCUIT_HPP
#define ADD_LINE_TO_CIRCUIT_HPP

#include <string>

#include <core/circuit.hpp>

namespace revkit
{

  /**
   * @brief Add a line to a circuit with specifying all meta-data
   *
   * This function helps adding a line to the circuit.
   * Besides incrementing the line counter, all meta-data information
   * is adjusted as well.
   *
   * @param circ Circuit
   * @param input Name of the input of the line
   * @param output Name of the output of the line
   * @param c Constant value of that line (Default: Not constant)
   * @param g If true, line is a garbage line
   *
   * @return The index of the newly added line
   *
   * @author RevKit
   * @since  1.0 (Return value since 1.1)
   */
  unsigned add_line_to_circuit( circuit& circ, const std::string& input, const std::string& output, const constant& c = constant(), bool g = false );

}

#endif /* ADD_LINE_TO_CIRCUIT_HPP */
