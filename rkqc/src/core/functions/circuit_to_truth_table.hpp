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
 * @file circuit_to_truth_table.hpp
 *
 * @brief Generates a truth table from a circuit
 */
#ifndef CIRCUIT_TO_TRUTH_TABLE_HPP
#define CIRCUIT_TO_TRUTH_TABLE_HPP

#include <boost/dynamic_bitset.hpp>
#include <boost/function.hpp>

#include <core/circuit.hpp>
#include <core/functor.hpp>
#include <core/truth_table.hpp>

namespace revkit
{

  /**
   * @brief Generates a truth table from a circuit
   *
   * This function takes a circuit, simulates it with a custom simulation function
   * and creates the specification. Further, the meta is copied.
   *
   * @param circ Circuit to be simulated
   * @param spec Empty truth table to be constructed
   * @param simulation Simulation function object
   *
   * @return true on success, false otherwise
   *
   * @author RevKit
   * @since  1.0
   */
  bool circuit_to_truth_table( const circuit& circ, binary_truth_table& spec, const functor<bool(boost::dynamic_bitset<>&, const circuit&, const boost::dynamic_bitset<>&)>& simulation );

}

#endif /* CIRCUIT_TO_TRUTH_TABLE_HPP */
