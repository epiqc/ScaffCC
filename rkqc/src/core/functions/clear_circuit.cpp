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

#include "clear_circuit.hpp"

namespace revkit
{

  void clear_circuit( circuit& circ )
  {
    circ.set_lines( 0 );
    circ.set_inputs( std::vector<std::string>() );
    circ.set_outputs( std::vector<std::string>() );
    circ.set_constants( std::vector<constant>() );
    circ.set_garbage( std::vector<bool>() );

    while ( circ.num_gates() )
    {
      circ.remove_gate_at( 0 );
    }
  }

}

