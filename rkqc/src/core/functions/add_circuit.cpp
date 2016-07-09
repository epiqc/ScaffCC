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

#include "add_circuit.hpp"

#include <boost/bind.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/insert.hpp>

namespace revkit
{

  void append_circuit( circuit& circ, const circuit& src, const gate::line_container& controls )
  {
    insert_circuit( circ, circ.num_gates(), src, controls );
  }

  void prepend_circuit( circuit& circ, const circuit& src, const gate::line_container& controls )
  {
    insert_circuit( circ, 0, src, controls );
  }

  void insert_circuit( circuit& circ, unsigned pos, const circuit& src, const gate::line_container& controls )
  {
    if ( controls.empty() )
    {
      foreach ( const gate& g, src )
      {
        circ.insert_gate( pos++ ) = g;
      }
    }
    else
    {
      foreach ( const gate& g, src )
      {
        gate& new_gate = circ.insert_gate( pos++ );
        boost::for_each( controls, boost::bind( &gate::add_control, &new_gate, _1 ) );
        std::for_each( g.begin_controls(), g.end_controls(), boost::bind( &gate::add_control, &new_gate, _1 ) );
        std::for_each( g.begin_targets(), g.end_targets(), boost::bind( &gate::add_target, &new_gate, _1 ) );
        new_gate.set_type( g.type() );
      }
    }
  }

}
