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
 * @file target_lines.hpp
 *
 * @brief Gets the target lines of a gate
 *
 */

#ifndef TARGET_LINES_HPP
#define TARGET_LINES_HPP

namespace revkit
{

  class gate;

  /**
   * @brief Gets the target lines of a gate
   *
   * This function stores all target lines of a gate into a container.
   *
   * @section Example
   * @code
   * gate g = ...;
   * std::vector<gate::line> targets;
   * target_lines( g, std::back_inserter( targets ) );
   * @endcode
   *
   * @param g      Gate
   * @param result Iterator to store the lines as gate::line type
   *
   * @return The iterator after adding the lines (pointing after the end)
   *
   * @author RevKit
   * @since  1.0
   */
  template<typename Iterator>
  Iterator target_lines( const gate& g, Iterator result )
  {
    for ( gate::const_iterator c = g.begin_targets(); c != g.end_targets(); ++c )
    {
      *result++ = *c;
    }
    return result;
  }
  
}

#endif /* TARGET_LINES_HPP */
