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

#include "write_specification.hpp"

#include <fstream>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/spirit/include/karma.hpp>

#include "../version.hpp"

#include "io_utils_p.hpp"

using namespace boost::assign;

#define foreach BOOST_FOREACH

namespace revkit
{

  struct tristate_to_char
  {
    char operator()( const binary_truth_table::value_type& vt ) const
    {
      return vt ? ( *vt ? '1' : '0' ) : '-';
    }
  };

  struct garbage_to_char
  {
    char operator()( bool g )
    {
      return g ? '1' : '-';
    }
  };

  write_specification_settings::write_specification_settings()
    : version( "2.0" ),
      header( boost::str( boost::format( "This file has been generated using RevKit %s (www.revkit.org)" ) % revkit_version() ) )
  {
  };

  bool write_specification( const binary_truth_table& spec, const std::string& filename, const write_specification_settings& settings )
  {
    std::filebuf fb;
    fb.open( filename.c_str(), std::ios::out );

    std::ostream os( &fb );

    if ( spec.num_inputs() < spec.num_outputs() )
    {
      return false;
    }

    unsigned oldsize = 0;

    if ( settings.header.size() )
    {
      std::string header = settings.header;
      boost::algorithm::replace_all( header, "\n", "\n# " );
      os << "# " << header << std::endl;
    }
    
    os << ".version " << settings.version << std::endl
       << ".numvars " << spec.num_inputs() << std::endl;

    std::vector<std::string> variables( spec.num_inputs() );

    for ( unsigned i = 0u; i < spec.num_inputs(); ++i )
    {
      variables[i] = boost::str( boost::format( "x%d" ) % i );
    }

    std::vector<std::string> _inputs = spec.inputs();
    oldsize = _inputs.size();
    _inputs.resize( spec.num_inputs() );

    for ( unsigned i = oldsize; i < spec.num_inputs(); ++i )
    {
      _inputs[i] = boost::str( boost::format( "i%d" ) % i );
    }

    std::vector<std::string> _outputs = spec.outputs();
    oldsize = _outputs.size();
    _outputs.resize( spec.num_inputs() );

    for ( unsigned i = oldsize; i < spec.num_inputs(); ++i )
    {
      _outputs[i] = boost::str( boost::format( "o%d" ) % i );
    }

    os << ".variables " << boost::algorithm::join( variables, " " ) << std::endl;

    namespace karma = boost::spirit::karma;
    namespace ascii = boost::spirit::ascii;

    os << ".inputs ";
    std::ostream_iterator<char> outit( os );
    karma::generate_delimited( outit, *( karma::no_delimit['"' << karma::string] << '"' ), ascii::space, _inputs );
    os << std::endl;

    os << ".outputs ";
    karma::generate_delimited( outit, *( karma::no_delimit['"' << karma::string] << '"' ), ascii::space, _outputs );
    os << std::endl;

    std::string _constants( spec.num_inputs(), '-' );
    std::transform( spec.constants().begin(), spec.constants().end(), _constants.begin(), tristate_to_char() );

    std::string _garbage( spec.num_inputs(), '-' );
    std::vector<bool> garbage = spec.garbage();
    std::transform( garbage.begin(), garbage.end(), _garbage.begin(), garbage_to_char() );

    os << ".constants " << _constants << std::endl
       << ".garbage " << _garbage << std::endl
       << ".begin" << std::endl;

    typedef std::map<unsigned, std::pair<binary_truth_table::out_const_iterator, binary_truth_table::out_const_iterator> > table_type;
    table_type table;

    for ( binary_truth_table::const_iterator it = spec.begin(); it != spec.end(); ++it )
    {
      std::vector<unsigned> numbers;

      in_cube_to_values( it->first.first, it->first.second, std::back_inserter( numbers ) );
    
      foreach ( unsigned& number, numbers )
      {
        table.insert( std::make_pair( number, it->second ) );
      }
    }

    table_type::const_iterator itTable = table.begin();
    unsigned position = 0;

    /* output permutation */
    std::vector<unsigned> output_order = settings.output_order;
    if ( output_order.size() != spec.num_outputs() )
    {
      output_order.clear();
      std::copy( boost::make_counting_iterator( 0u ), boost::make_counting_iterator( spec.num_outputs() ), std::back_inserter( output_order ) );
    }

    do
    {
      // fill free spaces
      unsigned to = ( itTable == table.end() ) ? ( 1u << spec.num_inputs() ) : itTable->first;

      for ( unsigned i = position; i < to; ++i )
      {
        os << std::string( spec.num_inputs(), '-' ) << std::endl;
      }

      // break if done
      if ( itTable == table.end() )
      {
        break;
      }

      // now the actual line
      std::string outLine( spec.num_inputs(), '-' );
      for ( binary_truth_table::out_const_iterator itOut = itTable->second.first; itOut != itTable->second.second; ++itOut )
      {
        outLine.at( output_order.at( itOut - itTable->second.first ) ) = tristate_to_char()( *itOut );
      }
      
      os << outLine << std::endl;

      position = to + 1;
      ++itTable;
      
    } while ( true );

    os << ".end" << std::endl;

    fb.close();

    return true;
  }

}
