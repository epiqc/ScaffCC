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

#include "truth_table.hpp"

namespace revkit
{

  std::ostream& operator<<( std::ostream& os, const binary_truth_table& spec )
  {
    for ( binary_truth_table::const_iterator it = spec.begin(); it != spec.end(); ++it )
    {
      // iterate through input cube (bit by bit)
      foreach ( const binary_truth_table::value_type& in_bit, it->first )
      {
        os << ( in_bit ? ( *in_bit ? "1" : "0" ) : "-" );
      }

      os << " ";

      // iterate through output cube (bit by bit)
      foreach ( const binary_truth_table::value_type& out_bit, it->second )
      {
        os << ( out_bit ? ( *out_bit ? "1" : "0" ) : "-" );
      }

      os << std::endl;
    }

    return os;
  }

  unsigned truth_table_cube_to_number( const binary_truth_table::cube_type& cube )
  {
    unsigned ret = 0;
    
    for ( unsigned i = 0; i < cube.size(); ++i )
    {
      assert( cube.at( i ) );
      ret |= ( *cube.at( i ) << ( cube.size() - 1 - i ) );
    }

    return ret;
  }

  binary_truth_table::cube_type number_to_truth_table_cube( unsigned number, unsigned bw )
  {
    binary_truth_table::cube_type c;

    for ( unsigned i = 0; i < bw; ++i )
    {
      c.push_back( ( number & ( 1u << ( bw -1  - i ) ) ) ? true : false );
    }

    return c;
  }

}
