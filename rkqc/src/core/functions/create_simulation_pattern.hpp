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
 * @file create_simulation_pattern.hpp
 *
 * @brief Create simulation pattern for sequential simulation
 *
 * @author RevKit
 * @since  1.2
 */

#ifndef CREATE_SIMULATION_PATTERN_HPP
#define CREATE_SIMULATION_PATTERN_HPP

#include <string>
#include <vector>
#include <map>

#include <boost/dynamic_bitset.hpp>

namespace revkit
{

  class circuit;
  class pattern;

  /**
   * @brief Create simulation pattern for sequential simulation
   *
   * @param p Pattern class
   * @param circ Circuit to be simulated
   * @param sim Empty vector which is filled with input assignments for every step
   * @param init Map which contains the initial assignment for state signals
   * @param error If not null, the string targeted by the pointer is assigned with an error message
   *
   * @author RevKit
   * @since  1.2
   */
  bool create_simulation_pattern( const pattern& p, const circuit& circ, std::vector<boost::dynamic_bitset<> >& sim, std::map<std::string, boost::dynamic_bitset<> >& init, std::string* error = 0 );

}

#endif /* CREATE_SIMULATION_PATTERN_HPP */

