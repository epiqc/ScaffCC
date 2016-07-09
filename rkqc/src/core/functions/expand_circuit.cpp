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

#include "expand_circuit.hpp"

#include <boost/tuple/tuple.hpp>

#include <core/functions/copy_circuit.hpp>

namespace revkit
{

  bool expand_circuit( const circuit& base, circuit& circ, unsigned num_lines, const std::vector<unsigned>& filter )
  {
    /* circ has to be empty */
    if ( circ.num_gates() != 0 )
    {
      assert( false );
      return false;
    }

    if ( num_lines == 0u || !filter.size() )
    {
      copy_circuit( base, circ );
      return true;
    }
    else
    {
      circ.set_lines( num_lines );

      foreach ( const gate& g, base )
      {
        gate& new_g = circ.append_gate();
      
        for ( gate::const_iterator it = g.begin_controls(); it != g.end_controls(); ++it )
        {
          new_g.add_control( filter.at( *it ) );
        }
        
        for ( gate::const_iterator it = g.begin_targets(); it != g.end_targets(); ++it )
        {
          new_g.add_target( filter.at( *it ) );
        }
        
        new_g.set_type( g.type() ); 
      }
    }

    return true;
  }

  bool expand_circuit( const circuit& base, circuit& circ )
  {
    /* circ has to be empty */
    if ( circ.num_gates() != 0 )
    {
      assert( false );
      return false;
    }

    /* is base a sub-circuit */
    if ( !base.is_subcircuit() )
    {
      copy_circuit( base, circ );
      return true;
    }

    return expand_circuit( base, circ, base.filter().first, base.filter().second );
  }

}

