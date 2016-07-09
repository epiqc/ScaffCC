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
 * @file transposition_to_circuit.hpp
 *
 * @brief A simple synthesis algorithm based on transpositions
 */
#ifndef TRANSPOSITION_TO_CIRCUIT
#define TRANSPOSITION_TO_CIRCUIT

#include <boost/dynamic_bitset.hpp>

namespace revkit
{
  class circuit;

  /**
   * @brief Creates a circuit realization for a transposition
   *
   * This function takes one line from a reversible truth table (inputs -> outputs)
   * and returns a circuit that will map this input assignment to the output assignment
   * and vice versa by leaving all other input and output assignments unaltered.
   *
   * @param circ An empty circuit that will be created. The number of lines of the circuit
   *             must coincide with the length of the input and output assignments.
   * @param inputs Input Assignment
   * @param outputs Output Assignment
   * @author RevKit
   * @since  1.3
   */
  bool transposition_to_circuit( circuit& circ,
      const boost::dynamic_bitset<>& inputs,
      const boost::dynamic_bitset<>& outputs);

}

#endif /* TRANSPOSITION_TO_CIRCUIT */
