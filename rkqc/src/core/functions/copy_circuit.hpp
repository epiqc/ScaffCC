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
 * @file copy_circuit.hpp
 *
 * @brief Copies a circuit
 *
 */

#ifndef COPY_CIRCUIT_HPP
#define COPY_CIRCUIT_HPP

#include <core/circuit.hpp>

namespace revkit
{

  /**
   * @brief Copies a circuit with all meta information
   *
   * This function creates a copy of the circuit \p src in \p dest
   * including all meta information as input and output names,
   * and also constant input and garbage output information.
   *
   * @param src  Source circuit
   * @param dest Destination circuit
   *
   * @author RevKit
   * @since  1.0
   */
  void copy_circuit( const circuit& src, circuit& dest );
  
}

#endif /* COPY_CIRCUIT_HPP */
