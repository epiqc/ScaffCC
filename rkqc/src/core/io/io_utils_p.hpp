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

/** @cond */
#ifndef IO_UTILS_P_HPP
#define IO_UTILS_P_HPP

#include <boost/assign/std/vector.hpp>

using namespace boost::assign;

namespace revkit
{
  
  template<typename Iterator>
  Iterator in_cube_to_values( binary_truth_table::in_const_iterator first, binary_truth_table::in_const_iterator last, Iterator result )
  {
    // Example
    // 0--11
    // base = 00011 = 3
    // dc_positions = 1,2
    // numbers = 3, 7, 11, 15

    unsigned base = 0;
    std::vector<unsigned> dc_positions;
    unsigned pos = 0;

    while ( first != last )
    {
      if ( *first ) // if not DC
      {
        base |= ( **first << ( last - first - 1 ) );
      }
      else
      {
        dc_positions += pos;
      }

      ++pos;
      ++first;
    }

    *result++ = base;

    for ( unsigned i = 1; i < ( 1u << dc_positions.size() ); ++i )
    {
      unsigned copy = base;
      for ( unsigned j = 0; j < dc_positions.size(); ++j )
      {
        unsigned local_bit = i & ( 1u << ( dc_positions.size() - j - 1 ) ) ? 1 : 0;
        unsigned global_bit_pos = pos - dc_positions.at( j ) - 1;
        copy |= ( local_bit << global_bit_pos );
      }
      *result++ = copy;
    }

    return result;
  }

}

#endif /* IO_UTILS_P_HPP */
/** @endcond */
