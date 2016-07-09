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
 * @file clear_circuit.hpp
 *
 * @brief Clears a circuit
 */

#ifndef CLEAR_CIRCUIT_HPP
#define CLEAR_CIRCUIT_HPP

#include "../circuit.hpp"

namespace revkit
{

  /**
   * @brief Clears the circuit \p circ
   *
   * This function clears all lines, gates and meta-data in a circuit.
   *
   * @param circ Circuit
   *
   * @author RevKit
   * @since  1.0
   */
  void clear_circuit( circuit& circ );

}

#endif /* CLEAR_CIRCUIT_HPP */
