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

#include "add_line_to_circuit.hpp"

#include <vector>

#include <boost/assign/std/vector.hpp>

using namespace boost::assign;

namespace revkit
{

  unsigned add_line_to_circuit( circuit& circ, const std::string& input, const std::string& output, const constant& c, bool g )
  {
    std::vector<std::string> ins = circ.inputs();
    std::vector<std::string> outs = circ.outputs();
    std::vector<constant> cs = circ.constants();
    std::vector<bool> gar = circ.garbage();

    circ.set_lines( circ.lines() + 1u );

    ins += input;
    circ.set_inputs( ins );
    
    outs += output;
    circ.set_outputs( outs );
    
    cs += c;
    circ.set_constants( cs );

    gar += g;
    circ.set_garbage( gar );

    return circ.lines() - 1u;
  }

}
