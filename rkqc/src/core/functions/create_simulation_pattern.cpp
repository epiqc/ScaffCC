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

#include "create_simulation_pattern.hpp"

#include <fstream>

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/assign/std/list.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/irange.hpp>

#include "../circuit.hpp"
#include "../pattern.hpp"

#define foreach BOOST_FOREACH

using namespace boost::assign;

namespace revkit
{

  bool create_simulation_pattern( const pattern& p, const circuit& circ, std::vector<boost::dynamic_bitset<> >& sim, std::map<std::string, boost::dynamic_bitset<> >& init, std::string* error )
  {
    // Initialzustand speichern
    const bus_collection::map& circStates = circ.statesignals().buses();
    foreach ( const pattern::initializer_map::value_type& pa, p.initializers() )
    {
      // is there a bus, we need it for the bitwidth
      bus_collection::map::const_iterator stateIt = circStates.find( pa.first );
      if ( stateIt != circStates.end() )
      {
        int busSize = stateIt->second.size();
        boost::dynamic_bitset<> initBits( busSize, pa.second );

        init.insert( std::make_pair( pa.first, initBits ) );
      }
    }

    // Set init value to 0 for all remaining state signals
    foreach ( const bus_collection::map::value_type& pa, circStates )
    {
      const std::string& statesignal = pa.first;
      std::map<std::string, boost::dynamic_bitset<> >::const_iterator stateIt = init.find( statesignal );
      if ( stateIt == init.end() )
      {
        init.insert( std::make_pair( statesignal, boost::dynamic_bitset<>( pa.second.size() ) ) );
      }
    }

    std::map<std::string, std::vector<unsigned> > varPlace;
    std::map<std::string, std::vector<unsigned> > varPlaceIC;
    std::list<unsigned> usedLines;
    unsigned numBits = 0;
    std::vector<std::string> vars;

    foreach ( const std::string& input_name, p.inputs() )
    {
      // Search in input busses
      bus_collection::map::const_iterator busIt = circ.inputbuses().buses().find( input_name );
      if( busIt != circ.inputbuses().buses().end() )
      {
        const std::vector<unsigned>& busLines = busIt->second;
        varPlaceIC[input_name] = busLines;
        foreach ( unsigned line, busLines )
        {
          usedLines += line;
          ++numBits;
        }
        vars += input_name;
      }
      // Search in regular inputs
      else
      {
        std::vector<std::string>::const_iterator itOtherInput = std::find( circ.inputs().begin(), circ.inputs().end(), input_name );

        if ( itOtherInput != circ.inputs().end() )
        {
          unsigned line = std::distance( circ.inputs().begin(), itOtherInput );
          varPlaceIC[input_name] = std::vector<unsigned>( 1, line );
          usedLines.push_back( line );
          ++numBits;
          vars.push_back( input_name );
        }
        // ignore if not found
      }
    }

    // check for multiple defined inputs
    usedLines.sort();
    std::list<unsigned> ul = usedLines;
    usedLines.unique();
    if ( usedLines.size() < ul.size() )
    {
      if( error )
      {
        *error = "At least one primary input is specified multiple times in .inputs.";
      }
      return false;
    }

    // check, whether all primary inputs are covered and nothing else
    std::list<unsigned> primaryInputs;
    boost::push_back( primaryInputs, boost::irange( 0u, circ.lines() ) );

    // remove constant lines
    for ( std::vector<constant>::const_iterator it = circ.constants().begin(); it != circ.constants().end(); ++it )
    {
      if ( *it )
      {
        primaryInputs.remove( std::distance( circ.constants().begin(), it ) );
      }
    }

    // remove statesignals
    foreach ( const bus_collection::map::value_type& pa, circStates )
    {

      foreach ( unsigned line, pa.second )
      {
        primaryInputs.remove( line );
      }
    }

    // check
    if ( primaryInputs != usedLines )
    {
      if ( error )
      {
        *error = "Specified inputs don't match primary inputs of the circuit.";
      }
      return false;
    }

    std::list<unsigned>::const_iterator listIt = usedLines.begin();
    foreach ( const std::string& varName, vars )
    {
      std::vector<unsigned> vp;
      foreach ( unsigned line, varPlaceIC[varName] )
      {
        vp += std::distance( usedLines.begin(), boost::find( usedLines, line ) );
      }
      varPlace.insert( std::make_pair( varName, vp ) );
    }

    for ( pattern::pattern_vec::const_iterator step = p.patterns().begin(); step != p.patterns().end(); ++step )
    {
      boost::dynamic_bitset<> stepBits(numBits);
      for( std::vector<unsigned>::const_iterator it2 = step->begin(); it2 != step->end(); ++it2 )
      {
        unsigned input = *it2;
        const std::vector<unsigned>& lines = varPlace[vars.at( std::distance( step->begin(), it2 ) )];
        if ( input >= ( 1u << lines.size() ) )
        {
          if ( error )
          {
            *error = boost::str( boost::format( "In step %d: Input %s has not enough bits to represent the given value." )
                                 % ( std::distance( p.patterns().begin(), step ) + 1u )
                                 % vars.at( std::distance( step->begin(), it2 ) ) );
          }
          return false;
        }

        boost::dynamic_bitset<> inputVal( lines.size(), input );
        unsigned j = 0u;
        foreach ( unsigned line, lines )
        {
          stepBits[line] = inputVal[j++];
        }
      }
      sim += stepBits;
    }

    return true;
  }

}

