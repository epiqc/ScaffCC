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

#include "print_statistics.hpp"

#include <boost/format.hpp>

#include "../utils/costs.hpp"

namespace revkit
{

  print_statistics_settings::print_statistics_settings()
    : main_template( "%1$sGates:            %2$d\nLines:            %3$d\nQuantum Costs:    %4$d\nTransistor Costs: %5$d\n" ),
      runtime_template( "Runtime:          %.2f\n" )
  {
  }

  void print_statistics( std::ostream& os, const circuit& circ, double runtime, const print_statistics_settings& settings )
  {
    std::string runtime_string;

    if ( runtime != -1 )
    {
      runtime_string = boost::str( boost::format( settings.runtime_template ) % runtime );
    }

    boost::format fmt( settings.main_template );
    fmt.exceptions( boost::io::all_error_bits ^ ( boost::io::too_many_args_bit | boost::io::too_few_args_bit ) );

    os << fmt % runtime_string % circ.num_gates() % circ.lines() % costs( circ, costs_by_gate_func( quantum_costs() ) ) % costs( circ, costs_by_gate_func( transistor_costs() ) );
  }

  void print_statistics( const circuit& circ, double runtime, const print_statistics_settings& settings )
  {
    print_statistics( std::cout, circ, runtime, settings );
  }

}
