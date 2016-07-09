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
 * @file flatten_circuit.hpp
 *
 * @brief Flattens a circuit with modules
 */

#ifndef FLATTEN_CIRCUIT_HPP
#define FLATTEN_CIRCUIT_HPP

namespace revkit
{

  class circuit;

  /**
   * @brief Flattens a circuit with modules
   *
   * This functions takes a circuit with module \p base and
   * substitutes all modules with their elementary gates
   * recursively and saves the result in \p circ.
   *
   * @param base Circuit to flatten (with modules)
   * @param circ Resulting circuit (without modules)
   * @param keep_meta_data Specifies, whether the RevLib 2.0 meta data such as buses should be kept (since 1.2)
   *
   * @author RevKit
   * @since  1.1
   */
  void flatten_circuit( const circuit& base, circuit& circ, bool keep_meta_data = false );

}

#endif /* FLATTEN_CIRCUIT_HPP */


