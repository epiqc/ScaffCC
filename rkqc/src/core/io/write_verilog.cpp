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

#include "write_verilog.hpp"

#include <sstream>

#include <boost/algorithm/string/join.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/format.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/none.hpp>
#include <boost/optional.hpp>

#include "../circuit.hpp"
#include "../target_tags.hpp"

using namespace boost::assign;

namespace revkit
{

  bool get_controls( std::vector<std::string>& controls, const gate& g, const std::vector<std::string>& current_signals, const std::vector<constant>& current_constants )
  {
    for ( gate::const_iterator it = g.begin_controls(); it != g.end_controls(); ++it )
    {
      if ( current_constants.at( *it ) ) // is constant
      {
        if ( *current_constants.at( *it ) ) // is constant 1
        {
          continue;
        }
        else // is constant 0
        {
          return false;
        }
      }
      else
      {
        controls += current_signals.at( *it );
      }
    }

    return true;
  }

  const std::string& add_wire( std::vector<std::string>& wires )
  {
    wires += boost::str( boost::format( "tmp%d" ) % wires.size() );
    return wires.back();
  }

  write_verilog_settings::write_verilog_settings()
    : propagate_constants( true )
  {
  }

  void write_verilog( const circuit& circ, std::ostream& os, const write_verilog_settings& settings )
  {
    using boost::adaptors::map_keys;
    using boost::adaptors::transformed;
    using boost::lambda::_1;

    // create a flip flop if there are state variables
    if ( !circ.statesignals().buses().empty() )
    {
      os << "module DFF( CK, D, Q );" << std::endl
         << "  input CK;" << std::endl
         << "  input D;" << std::endl
         << "  output Q;" << std::endl << std::endl
         << "  reg ff;" << std::endl
         << "  buf( ff, D );" << std::endl
         << "  buf( Q, ff );" << std::endl
         << "endmodule" << std::endl << std::endl;
    }

    // main module body code
    std::stringstream body;

    std::vector<std::string> current_signals( circ.lines() );
    std::vector<constant>    current_constants( circ.lines(), constant() );

    std::vector<std::string> wires;

    std::copy( circ.inputs().begin(), circ.inputs().end(), current_signals.begin() );

    if ( settings.propagate_constants )
    {
      std::copy( circ.constants().begin(), circ.constants().end(), current_constants.begin() );
    }

    // adjust inputs: states
    {
      foreach ( const bus_collection::map::value_type& bus, circ.statesignals().buses() )
      {
        for ( unsigned i = 0u; i < bus.second.size(); ++i )
        {
          std::replace( current_signals.begin(), current_signals.end(), circ.inputs().at( bus.second.at( i ) ), boost::str( boost::format( "%s_in[%d]" ) % bus.first % i ) );
        }
      }
    }
    
    // adjust inputs: blocks
    {
      foreach ( const bus_collection::map::value_type& bus, circ.inputbuses().buses() )
      {
        for ( unsigned i = 0u; i < bus.second.size(); ++i )
        {
          std::replace( current_signals.begin(), current_signals.end(), circ.inputs().at( bus.second.at( i ) ), boost::str( boost::format( "%s[%d]" ) % bus.first % i ) );
        }
      }
    }

    // adjust inputs: constants
    if ( !settings.propagate_constants )
    {
      body << "  // constant assumptions: ";
      std::vector<std::string> const_assignments;
      for ( unsigned i = 0u; i < circ.lines(); ++i )
      {
        if ( circ.constants().at( i ) )
        {
          if ( circ.inputs().at( i ) == "0" || circ.inputs().at( i ) == "1" )
          {
            current_signals.at( i ) = boost::str( boost::format( "constant%d_%d" ) % ( *circ.constants().at( i ) ) % i );
          }

          const_assignments += boost::str( boost::format( "%s == %d" ) % current_signals.at( i ) % *circ.constants().at( i ) );
        }
      }
      body << boost::algorithm::join( const_assignments, " && " ) << std::endl;
    }

    // adjust inputs: force bad Verilog Code when using constants directly
    for ( unsigned i = 0u; i < current_constants.size(); ++i )
    {
      if ( current_constants.at( i ) )
      {
        current_signals.at( i ).clear();
      }
    }

    unsigned pos = 0u;
    foreach ( const gate& g, circ )
    {
      body << "  // gate " << pos++ << std::endl; 

      if ( is_toffoli( g ) )
      {
        unsigned target_pos = *g.begin_targets();
        std::string target_signal = current_signals.at( target_pos );
        constant target_constant = current_constants.at( target_pos );

        std::vector<std::string> controls;
        if ( get_controls( controls, g, current_signals, current_constants ) )
        {
          switch ( controls.size() )
          {
          case 0:
            if ( target_constant )
            {
              current_constants.at( target_pos ) = !*target_constant;
              continue;
            }
            else
            {
              body << "  not( " << add_wire( wires ) << ", " << target_signal << " );" << std::endl;
            }
            break;

          case 1:
            if ( target_constant )
            {
              body << "  " << ( *target_constant ? "not" : "buf" ) << "( " << add_wire( wires ) << ", " << controls.front() << " );" << std::endl;
            }
            else
            {
              body << "  xor( " << add_wire( wires ) << ", " << target_signal << ", " << controls.front() << " );" << std::endl;
            }
            break;

          default:
            {
              while ( controls.size() > 2u )
              {
                std::string tmp_wire = add_wire( wires );
                body << "  and( " << tmp_wire << ", " << controls.at( 0u ) << ", " << controls.at( 1u ) << " );" << std::endl;
                controls.erase( controls.begin() );
                controls.erase( controls.begin() );
                controls.insert( controls.begin(), tmp_wire );
              }

              std::string and_wire = add_wire( wires );
              body << "  and( " << and_wire << ", " << boost::algorithm::join( controls, ", " ) << " );" << std::endl;

              if ( target_constant )
              {
                body << "  " << ( *target_constant ? "not" : "buf" ) << "( " << add_wire( wires ) << ", " << and_wire << " );" << std::endl;
              }
              else
              {
                body << "  xor( " << add_wire( wires ) << ", " << target_signal << ", " << and_wire << " );" << std::endl;
              }
              break;
            }
          }

          // update current_signals and current_constants
          current_signals.at( target_pos ) = wires.back();
          current_constants.at( target_pos ) = boost::none;
        }
      }
      else if ( is_fredkin( g ) )
      {
        gate::const_iterator it = g.begin_targets();

        unsigned target_pos1 = *it++;
        unsigned target_pos2 = *it;
        std::string target_signal1 = current_signals.at( target_pos1 );
        std::string target_signal2 = current_signals.at( target_pos2 );
        constant target_constant1 = current_constants.at( target_pos1 );
        constant target_constant2 = current_constants.at( target_pos2 );

        std::vector<std::string> controls;
        if ( get_controls( controls, g, current_signals, current_constants ) )
        {
          std::string new_target1;
          std::string new_target2;

          switch ( controls.size() )
          {
          case 0:
            if ( target_constant1 )
            {
              current_constants.at( target_pos2 ) = *target_constant1;
            }
            else
            {
              new_target2 = add_wire( wires );
              body << "  buf( " << new_target2 << ", " << target_signal1 << std::endl;
            }

            if ( target_constant2 )
            {
              current_constants.at( target_pos1 ) = *target_constant2;
            }
            else
            {
              new_target1 = add_wire( wires );
              body << "  buf( " << new_target1 << ", " << target_signal2 << std::endl;
            }
            break;

          case 1:
            if ( target_constant1 && !target_constant2 )
            {
              // select inverse
              std::string not_controls = add_wire( wires );
              body << "  not( " << not_controls << ", " << controls.front() << " );" << std::endl;

              new_target1 = add_wire( wires );
              new_target2 = add_wire( wires );

              body << "  " << ( *target_constant1 ? "or" : "and" ) << "( " << new_target1 << ( *target_constant1 ? not_controls : controls.front() ) << ", " << target_signal2 << " );" << std::endl;
              body << "  " << ( *target_constant1 ? "or" : "and" ) << "( " << new_target2 << ( *target_constant1 ? controls.front() : not_controls ) << ", " << target_signal2 << " );" << std::endl;
            }
            else if ( !target_constant1 && target_constant2 )
            {
              // select inverse
              std::string not_controls = add_wire( wires );
              body << "  not( " << not_controls << ", " << controls.front() << " );" << std::endl;

              new_target1 = add_wire( wires );
              new_target2 = add_wire( wires );

              body << "  " << ( *target_constant2 ? "or" : "and" ) << "( " << new_target1 << ", " << ( *target_constant2 ? controls.front() : not_controls ) << ", " << target_signal1 << " );" << std::endl;
              body << "  " << ( *target_constant2 ? "or" : "and" ) << "( " << new_target2 << ", " << ( *target_constant2 ? not_controls : controls.front() ) << ", " << target_signal1 << " );" << std::endl;
            }
            else if ( target_constant1 && target_constant2 )
            {
              // only consider if constants are different
              if ( *target_constant1 != *target_constant2 )
              {
                // select inverse
                std::string not_controls = add_wire( wires );
                body << "  not( " << not_controls << ", " << controls.front() << " );" << std::endl;

                new_target1 = add_wire( wires );
                new_target2 = add_wire( wires );

                body << "  buf(" << new_target1 << ", " << ( *target_constant1 ? not_controls : controls.front() ) << std::endl;
                body << "  buf(" << new_target2 << ", " << ( *target_constant1 ? controls.front() : not_controls ) << std::endl;
              }
            }
            else
            {
              // select
              std::string ctrls = controls.front();

              // select inverse
              std::string not_ctrls = add_wire( wires );
              body << boost::format( "  not( %s, %s );" ) % not_ctrls % ctrls << std::endl;

              // products
              std::string ct1  = add_wire( wires );
              std::string ct2  = add_wire( wires );
              std::string nct1 = add_wire( wires );
              std::string nct2 = add_wire( wires );

              body << boost::format( "  and( %s, %s, %s );" ) % ct1  % ctrls % target_signal1 << std::endl;
              body << boost::format( "  and( %s, %s, %s );" ) % ct2  % ctrls % target_signal2 << std::endl;
              body << boost::format( "  and( %s, %s, %s );" ) % nct1 % not_ctrls % target_signal1 << std::endl;
              body << boost::format( "  and( %s, %s, %s );" ) % nct2 % not_ctrls % target_signal2 << std::endl;

              new_target1 = add_wire( wires );
              new_target2 = add_wire( wires );

              body << boost::format( "  or( %s, %s, %s );" ) % new_target1 % ct2 % nct1 << std::endl;
              body << boost::format( "  or( %s, %s, %s );" ) % new_target2 % ct1 % nct2 << std::endl;
            }
            break;

          default:
            {
              // select
              while ( controls.size() > 2u )
              {
                std::string tmp_wire = add_wire( wires );
                body << "  and( " << tmp_wire << ", " << controls.at( 0u ) << ", " << controls.at( 1u ) << " );" << std::endl;
                controls.erase( controls.begin() );
                controls.erase( controls.begin() );
                controls.insert( controls.begin(), tmp_wire );
              }

              std::string ctrls = add_wire( wires );
              body << "  and( " << ctrls << ", " << boost::algorithm::join( controls, ", " ) << " );" << std::endl;            

              // select inverse
              std::string not_ctrls = add_wire( wires );
              body << boost::format( "  not( %s, %s );" ) % not_ctrls % ctrls << std::endl;

              if ( target_constant1 && !target_constant2 )
              {
                new_target1 = add_wire( wires );
                new_target2 = add_wire( wires );

                body << "  " << ( *target_constant1 ? "or" : "and" ) << "( " << new_target1 << ", " << ( *target_constant1 ? not_ctrls : ctrls ) << ", " << target_signal2 << " );" << std::endl;
                body << "  " << ( *target_constant1 ? "or" : "and" ) << "( " << new_target2 << ", " << ( *target_constant1 ? ctrls : not_ctrls ) << ", " << target_signal2 << " );" << std::endl;
              }
              else if ( !target_constant1 && target_constant2 )
              {
                new_target1 = add_wire( wires );
                new_target2 = add_wire( wires );

                body << "  " << ( *target_constant2 ? "or" : "and" ) << "( " << new_target1 << ", " << ( *target_constant2 ? ctrls : not_ctrls ) << ", " << target_signal1 << " );" << std::endl;
                body << "  " << ( *target_constant2 ? "or" : "and" ) << "( " << new_target2 << ", " << ( *target_constant2 ? not_ctrls : ctrls ) << ", " << target_signal1 << " );" << std::endl;
              }
              else if ( target_constant1 && target_constant2 )
              {
                // only consider if constants are different
                if ( *target_constant1 != *target_constant2 )
                {
                  new_target1 = add_wire( wires );
                  new_target2 = add_wire( wires );
                  
                  body << "  buf(" << new_target1 << ", " << ( *target_constant1 ? not_ctrls : ctrls ) << std::endl;
                  body << "  buf(" << new_target2 << ", " << ( *target_constant1 ? ctrls : not_ctrls ) << std::endl;
                }
              }
              else
              {
                // products
                std::string ct1  = add_wire( wires );
                std::string ct2  = add_wire( wires );
                std::string nct1 = add_wire( wires );
                std::string nct2 = add_wire( wires );

                body << boost::format( "  and( %s, %s, %s );" ) % ct1  % ctrls % target_signal1 << std::endl;
                body << boost::format( "  and( %s, %s, %s );" ) % ct2  % ctrls % target_signal2 << std::endl;
                body << boost::format( "  and( %s, %s, %s );" ) % nct1 % not_ctrls % target_signal1 << std::endl;
                body << boost::format( "  and( %s, %s, %s );" ) % nct2 % not_ctrls % target_signal2 << std::endl;

                new_target1 = add_wire( wires );
                new_target2 = add_wire( wires );

                body << boost::format( "  or( %s, %s, %s );" ) % new_target1 % ct2 % nct1 << std::endl;
                body << boost::format( "  or( %s, %s, %s );" ) % new_target2 % ct1 % nct2 << std::endl;
              }
              break;
            }
          }

          // update current_signals and current_constants
          if ( !new_target1.empty() )
          {
            current_signals.at( target_pos1 ) = new_target1;
            current_constants.at( target_pos1 ) = boost::none;
          }

          if ( !new_target2.empty() )
          {
            current_signals.at( target_pos2 ) = new_target2;
            current_constants.at( target_pos2 ) = boost::none;
          }
        }
      }
      else
      {
        // TODO implement other gate types
        assert( false );
      }
    }

    // write top module code
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;

    // inputs and outputs
    for ( unsigned i = 0u; i < circ.lines(); ++i )
    {
      if ( ( !circ.constants().at( i ) || !settings.propagate_constants ) && circ.statesignals().find_bus( i ).empty() && circ.inputbuses().find_bus( i ).empty() )
      {
        std::string name = circ.inputs().at( i );

        if ( name == "0" || name == "1" )
        {
          name = boost::str( boost::format( "constant%s_%d" ) % name % i );
        }

        inputs += name;
      }

      if ( !circ.garbage().at( i ) && circ.statesignals().find_bus( i ).empty() && circ.outputbuses().find_bus( i ).empty() )
      {
        outputs += circ.outputs().at( i );
      }
    }

    boost::function<std::string(std::string)> append = ( _1 + boost::lambda::constant( "_out" ) );

    std::vector<std::string> signals;
    boost::push_back( signals, inputs );
    boost::push_back( signals, circ.inputbuses().buses() | map_keys );
    boost::push_back( signals, outputs );
    boost::push_back( signals, circ.outputbuses().buses() | map_keys );
    boost::push_back( signals, circ.statesignals().buses() | map_keys | transformed( append ) );

    os << "module top( clk, " << boost::algorithm::join( signals, ", " ) << " );" << std::endl;

    os << "  input clk;" << std::endl;
    if ( inputs.size() )
    {
      os << "  input " << boost::algorithm::join( inputs, ", " ) << ";" << std::endl;
    }

    foreach ( const bus_collection::map::value_type& bus, circ.inputbuses().buses() )
    {
      os << boost::format( "  input [%d:0] %s;" ) % ( bus.second.size() - 1u ) % bus.first << std::endl;
    }

    if ( outputs.size() )
    {
      os << "  output " << boost::algorithm::join( outputs, ", " ) << ";" << std::endl;
    }

    foreach ( const bus_collection::map::value_type& bus, circ.outputbuses().buses() )
    {
      os << boost::format( "  output [%d:0] %s;" ) % ( bus.second.size() - 1u ) % bus.first << std::endl;
    }

    os << "  wire " << boost::algorithm::join( wires, ", " ) << ";" << std::endl;

    foreach ( const bus_collection::map::value_type& bus, circ.statesignals().buses() )
    {
      os << boost::format( "  wire [%d:0] %s_in;" ) % ( bus.second.size() - 1u ) % bus.first << std::endl;
    }

    foreach ( const bus_collection::map::value_type& bus, circ.statesignals().buses() )
    {
      os << boost::format( "  output [%d:0] %s_out;" ) % ( bus.second.size() - 1u ) % bus.first << std::endl;
    }

    os << body.str() << std::endl;

    // map outputs
    os << "  // map outputs" << std::endl;
    for ( unsigned i = 0u; i < circ.lines(); ++i )
    {
      if ( !circ.garbage().at( i ) )
      {
        std::string output_signal = circ.outputs().at( i );

        // is in block?
        std::string bus_name = circ.outputbuses().find_bus( i );
        if ( !bus_name.empty() )
        {
          const std::vector<unsigned>& line_indices = circ.outputbuses().get( bus_name );
          unsigned pos_in_block = std::find( line_indices.begin(), line_indices.end(), i ) - line_indices.begin();
          output_signal = boost::str( boost::format( "%s[%d]" ) % bus_name % pos_in_block );
        }

        // is state
        std::vector<unsigned>::const_iterator itState;
        if ( circ.statesignals().has_bus( i ) )
        {
          output_signal = boost::str( boost::format( "%s_out[%d]" ) % circ.statesignals().find_bus( i ) % circ.statesignals().signal_index( i ) );
        }

        os << boost::format( "  buf( %s, %s );" ) % output_signal % current_signals.at( i ) << std::endl;
      }
    }

    // map state signals
    os << std::endl << "  // map state signals" << std::endl;
    foreach ( const bus_collection::map::value_type& bus, circ.statesignals().buses() )
    {
      for ( unsigned i = 0u; i < bus.second.size(); ++i )
      {
        os << boost::format( "  DFF( clk, %1%_out[%2%], %1%_in[%2%] );" ) % bus.first % i << std::endl;
      }
    }
    
    os << "endmodule" << std::endl;
  }

}
