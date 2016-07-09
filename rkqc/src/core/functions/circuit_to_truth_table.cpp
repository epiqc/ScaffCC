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

#include "circuit_to_truth_table.hpp"

#include <boost/format.hpp>

#include <core/properties.hpp>

namespace revkit
{

  void bitset_to_vector( binary_truth_table::cube_type& vec, const boost::dynamic_bitset<> number )
  {
    vec.clear();
    for ( unsigned i = 0u; i < number.size(); ++i )
    {
      vec.push_back( number.test( i ) );
    }
  }

  boost::dynamic_bitset<>& inc( boost::dynamic_bitset<>& bitset )
  {
    for ( boost::dynamic_bitset<>::size_type i = 0; i < bitset.size(); ++i )
    {
      bitset.flip( i );
      if ( bitset.test( i ) ) break;
    }
    return bitset;
  }

  bool circuit_to_truth_table( const circuit& circ, binary_truth_table& spec, const functor<bool(boost::dynamic_bitset<>&, const circuit&, const boost::dynamic_bitset<>&)>& simulation )
  {
    // number of patterns to check depends on partial or non-partial simulation
    boost::dynamic_bitset<>::size_type n = ( simulation.settings() && simulation.settings()->get<bool>( "partial", false ) ) ? std::count( circ.constants().begin(), circ.constants().end(), constant() ) : circ.lines();
    boost::dynamic_bitset<> input( n, 0u );

    do
    {
      boost::dynamic_bitset<> output;
      
      if ( simulation( output, circ, input ) )
      {
        binary_truth_table::cube_type in_cube, out_cube;
       
        bitset_to_vector( in_cube, input );
        bitset_to_vector( out_cube, output );
        
        spec.add_entry( in_cube, out_cube );
      }
      else
      {
        return false;
      }
    } while ( !inc( input ).none() );

    // metadata
    spec.set_inputs( circ.inputs() );
    spec.set_outputs( circ.outputs() );
    spec.set_constants( circ.constants() );
    spec.set_garbage( circ.garbage() );

    return true;
  }

}
