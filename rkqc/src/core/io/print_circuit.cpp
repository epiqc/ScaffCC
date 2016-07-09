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

#include "print_circuit.hpp"

#include <boost/algorithm/string/trim.hpp>
#include <boost/format.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/regex.hpp>

#include <boost/tuple/tuple.hpp>

#include "../circuit.hpp"
#include "../target_tags.hpp"

namespace revkit
{

  print_circuit_settings::print_circuit_settings( std::ostream& os )
    : os( os ),
      print_inputs_and_outputs( false ),
      print_gate_index( false ),
      control_char( '*' ),
      line_char( '-' ),
      gate_spacing( 0 ),
      line_spacing( 0 )
  {
  }

  print_circuit_settings::~print_circuit_settings()
  {
  }

  char print_circuit_settings::target_type_char( const gate& g ) const
  {
    if ( is_toffoli( g ) || is_peres( g ) || is_cnot( g ) )
    {
      return 'O';
    }
    else if ( is_fredkin( g ) )
    {
      return  'x';
    }
    else if ( is_v( g ) )
    {
      return 'v';
    }
    else if ( is_vplus( g ) )
    {
      return '+';
    }
    else if ( is_module( g ) )
    {
      return 'M';
    }
    else
    {
      return ' ';
    }
  }

  struct string_size_compare
  {
    bool operator()( const std::string& a, const std::string& b ) const
    {
      return a.size() < b.size();
    }
  };

  void print_circuit( const circuit& circ, const print_circuit_settings& settings )
  {
    if ( circ.num_gates() == 0 || circ.lines() == 0 )
    {
      return;
    }

    // when printing inputs and outputs we need to find the maximum size
    unsigned longest_input_length = 0;

    if ( settings.print_inputs_and_outputs )
    {
      longest_input_length = std::max_element( circ.inputs().begin(), circ.inputs().end(), string_size_compare() )->size();
    }

    // each element is a line with elements of (gate_index,character)
    std::vector<std::vector<boost::tuples::tuple<unsigned,char> > > line_chars( circ.lines() );

    // iterate over all gates
    for ( circuit::const_iterator itGate = circ.begin(); itGate != circ.end(); ++itGate )
    {
      unsigned gate_index = itGate - circ.begin();

      // iterate over all controls in the gate
      for ( gate::const_iterator itControl = (*itGate).begin_controls(); itControl != (*itGate).end_controls(); ++itControl )
      {
        if ( *itControl < line_chars.size() )
        {
          line_chars[*itControl].push_back( boost::tuples::make_tuple( gate_index, settings.control_char ) );
        }
      }

      // iterate over all targets in the gate
      for ( gate::const_iterator itTarget = (*itGate).begin_targets(); itTarget != (*itGate).end_targets(); ++itTarget )
      {
        if ( *itTarget < line_chars.size() )
        {
          line_chars[*itTarget].push_back( boost::tuples::make_tuple( gate_index, settings.target_type_char( *itGate ) ) );
        }
      }
    }

    if ( settings.print_gate_index )
    {
      if ( settings.print_inputs_and_outputs )
      {
        settings.os << std::string( longest_input_length + 1, ' ' );
      }

      using boost::lambda::_1;
      std::for_each( boost::counting_iterator<unsigned>( 0u ), boost::counting_iterator<unsigned>( circ.num_gates() ), settings.os << ( _1 % 10 ) );
      settings.os << std::endl;
    }

    for ( unsigned i = 0; i < circ.lines(); ++i )
    {
      std::string line_str( circ.num_gates(), settings.line_char );

      for ( std::vector<boost::tuples::tuple<unsigned, char> >::const_iterator it = line_chars[i].begin(); it != line_chars[i].end(); ++it ) {
        line_str[boost::tuples::get<0>( *it )] = boost::tuples::get<1>( *it );
      }

      if ( settings.gate_spacing )
      {
        std::string spaces( settings.gate_spacing, ' ' );
        line_str = boost::regex_replace( line_str, boost::regex( "(.)" ), boost::str( boost::format( "\\1%s" ) % spaces ) );
        boost::algorithm::trim( line_str );
      }

      if ( settings.print_inputs_and_outputs )
      {
        settings.os << std::string( longest_input_length - circ.inputs().at( i ).size(), ' ' ) << circ.inputs().at( i ) << " ";
      }

      settings.os << line_str;

      if ( settings.print_inputs_and_outputs )
      {
        settings.os << " " << circ.outputs().at( i );
      }

      settings.os << std::endl;

      for ( unsigned j = 0; j < settings.line_spacing; ++j )
      {
        settings.os << std::endl;
      }
    }
  }

  std::ostream& operator<<( std::ostream& os, const circuit& circ )
  {
    print_circuit_settings settings( os );
    print_circuit( circ, settings );
    return os;
  }

}
