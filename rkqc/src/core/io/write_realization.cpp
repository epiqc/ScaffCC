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

#include "write_realization.hpp"

#include <fstream>

#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/spirit/include/karma.hpp>

#include "../circuit.hpp"
#include "../target_tags.hpp"
#include "../version.hpp"

#define foreach BOOST_FOREACH

using namespace boost::assign;

namespace revkit
{

  struct constant_to_char
  {
    char operator()( const constant& c )
    {
      return ( c ? ( *c ? '1' : '0' ) : '-' );
    }
  };

  struct garbage_to_char
  {
    char operator()( bool g )
    {
      return g ? '1' : '-';
    }
  };

  struct line_to_variable
  {
    std::string operator()( const gate::line& l ) const
    {
//        return circ.inputs()[l];
      return boost::str( boost::format( "x%d" ) % l );
    }
  };

  write_realization_settings::write_realization_settings()
    : version( "2.0" ),
      header( boost::str( boost::format( "This file has been generated using RevKit %s (www.revkit.org)" ) % revkit_version() ) )
  {
  }

  void write_realization( const circuit& circ, std::ostream& os, const write_realization_settings& settings )
  {
    unsigned oldsize = 0;

    if ( !settings.header.empty() )
    {
      std::string header = settings.header;
      boost::algorithm::replace_all( header, "\n", "\n# " );
//      os << "# " << header << std::endl;
    }

    if ( !settings.version.empty() )
    {
//      os << ".version " << settings.version << std::endl;
    }

//    os << ".numvars " << circ.lines() << std::endl;

    std::vector<std::string> variables( circ.lines() );
    unsigned int total_lines = circ.lines();
    std::vector<std::string> _inputs( circ.inputs().begin(), circ.inputs().end() );
	std::vector<std::string> _workers( circ.workers.begin(), circ.workers.end() );
	std::vector<std::string> _ancilla_zg( circ.anc_zg.begin(), circ.anc_zg.end() );
	std::vector<std::string> _ancilla_zz( circ.anc_zz.begin(), circ.anc_zz.end() );
	std::vector<std::string> _ancilla_1g( circ.anc_1g.begin(), circ.anc_1g.end() );
	std::vector<std::string> _ancilla_11( circ.anc_11.begin(), circ.anc_11.end() );

//	for ( unsigned i = 0; i < circ.lines(); i++)
//	{
//		qubits[i] = boost::lexical_cast<std::string>(i); 
//	}

    oldsize = _inputs.size();
    _inputs.resize( circ.lines() );

    for ( unsigned i = oldsize; i < circ.lines(); ++i )
    {
      _inputs[i] = boost::str( boost::format( "i%d" ) % i );
    }

    std::vector<std::string> _outputs( circ.outputs().begin(), circ.outputs().end() );
    oldsize = _outputs.size();
    _outputs.resize( circ.lines() );

    for ( unsigned i = oldsize; i < circ.lines(); ++i )
    {
      _outputs[i] = boost::str( boost::format( "o%d" ) % i );
    }

//    os << ".variables " << boost::algorithm::join( variables, " " ) << std::endl;
  
//    foreach ( std::string var, _workers ) 
//    {
//        os << "qubit " << var << std::endl; 
//    }

    unsigned int k = 0;
    while( k < total_lines )
	{
		os << "qubit " << circ.lines_to_inputs.find(k)->second << std::endl;
        k++;
	}

//	foreach ( std::string var, _ancilla_zg )
//	{
//		os << "qubit " << var << std::endl;
//	}
//	foreach ( std::string var, _ancilla_zz )
//	{
//		os << "qubit " << var << std::endl;
//	}
//	foreach ( std::string var, _ancilla_1g )
//	{
//		os << "qubit " << var << std::endl;
//	}
//	foreach ( std::string var, _ancilla_11 )
//	{
//		os << "qubit " << var << std::endl;
//	}





    namespace karma = boost::spirit::karma;
    namespace ascii = boost::spirit::ascii;

//    os << ".inputs";
    //std::ostream_iterator<char> outit( os );
    //karma::generate_delimited( outit, *( karma::no_delimit['"' << karma::string] << '"' ), ascii::space, _inputs );

    foreach ( const std::string& _input, _inputs )
    {
      std::string quote = ( _input.find( " " ) != std::string::npos ) ? "\"" : "";
//      os << boost::format( " %s%s%s" ) % quote % _input % quote;
    }

//    os << std::endl;

//    os << ".outputs ";
    //karma::generate_delimited( outit, *( karma::no_delimit['"' << karma::string] << '"' ), ascii::space, _outputs );

    foreach ( const std::string& _output, _outputs )
    {
      std::string quote = ( _output.find( " " ) != std::string::npos ) ? "\"" : "";
//      os << boost::format( " %s%s%s" ) % quote % _output % quote;
    }

//    os << std::endl;

    std::string _constants( circ.lines(), '-' );
    std::transform( circ.constants().begin(), circ.constants().end(), _constants.begin(), constant_to_char() );

    std::string _garbage( circ.lines(), '-' );
    std::transform( circ.garbage().begin(), circ.garbage().end(), _garbage.begin(), garbage_to_char() );

//    os << ".constants " << _constants << std::endl
//       << ".garbage " << _garbage << std::endl;

    foreach ( const bus_collection::map::value_type& bus, circ.inputbuses().buses() )
    {
      std::vector<std::string> lines;
      std::transform( bus.second.begin(), bus.second.end(), std::back_inserter( lines ), line_to_variable() );
//      os << ".inputbus " << bus.first << " " << boost::algorithm::join( lines, " " ) << std::endl;
    }

    foreach ( const bus_collection::map::value_type& bus, circ.outputbuses().buses() )
    {
      std::vector<std::string> lines;
      std::transform( bus.second.begin(), bus.second.end(), std::back_inserter( lines ), line_to_variable() );
      os << ".outputbus " << bus.first << " " << boost::algorithm::join( lines, " " ) << std::endl;
    }

    foreach ( const bus_collection::map::value_type& bus, circ.statesignals().buses() )
    {
      std::vector<std::string> lines;
      std::transform( bus.second.begin(), bus.second.end(), std::back_inserter( lines ), line_to_variable() );
      os << ".state " << bus.first << " " << boost::algorithm::join( lines, " " ) << std::endl;
    }

    typedef std::pair<std::string, boost::shared_ptr<circuit> > pair_t;
    foreach ( const pair_t& module, circ.modules() )
    {
      os << ".module " << module.first << std::endl;

      write_realization_settings module_settings;
      module_settings.version.clear();
      module_settings.header.clear();
      write_realization( *module.second, os, module_settings );
    }

//    os << ".begin" << std::endl;

    std::string cmd;

    foreach ( const gate& g, circ )
    {

      if ( is_cnot( g ) )
      {
        cmd = "cnot";
      }
      else if ( is_not( g ) )
      {
        cmd = "X";
      }
      else if ( is_toffoli( g ) )
      {
        cmd = "toffoli";
//        cmd = boost::str( boost::format( "t%d" ) % g.size() );
      }
      else if ( is_fredkin( g ) )
      {
        cmd = boost::str( boost::format( "f%d" ) % g.size() );
      }
      else if ( is_peres( g ) )
      {
        cmd = "p";
      }
      else if ( is_v( g ) )
      {
        cmd = "v";
      }
      else if ( is_vplus( g ) )
      {
        cmd = "v+";
      }
      else if ( is_module( g ) )
      {
        cmd = boost::any_cast<module_tag>( g.type() ).name;
      }

      std::vector<std::string> lines;
      std::vector<unsigned> target_list;

      std::vector<unsigned> targets( g.begin_targets(), g.end_targets() );
      std::vector<unsigned> targets_o ( g.targets_ordered() );
      std::vector<unsigned> controls( g.begin_controls(), g.end_controls() );
      std::vector<unsigned> controls_o ( g.controls_ordered() );
      
/*
	  int j;
	  for( j=0; j < controls_o.size(); j++)
	  {
	  	 os << boost::lexical_cast<std::string>(g.controls_ordered[j]) << " "; 
//	  	 os << boost::lexical_cast<std::string>(controls[j]) << std::endl;
	  }
	  os << std::endl;
*/



      // Peres is special
      if ( is_peres( g ) )
      {
        std::transform( g.begin_controls(), g.end_controls(), std::back_inserter( lines ), line_to_variable() );


        if ( boost::any_cast<peres_tag>( &g.type() )->swap_targets )
        {
          std::reverse( targets.begin(), targets.end() );
        }
        std::transform( targets.begin(), targets.end(), std::back_inserter( lines ), line_to_variable() );
      }
      else if ( !is_module( g ) )
      {
        std::transform( g.begin_controls(), g.end_controls(), std::back_inserter( lines ), line_to_variable() );
        std::transform( g.begin_targets(), g.end_targets(), std::back_inserter( lines ), line_to_variable() );
      }

      else
      {
        std::transform( g.begin_controls(), g.end_controls(), std::back_inserter( lines ), line_to_variable() );

        foreach ( unsigned index, boost::any_cast<module_tag>( g.type() ).target_sort_order )
        {
          lines += line_to_variable()( targets.at( index ) );
          target_list += targets.at( index );
        }
      }
    
      std::vector<std::string> control_target_names;
      
      foreach ( unsigned ct, controls_o )
      {
          control_target_names += circ.lines_to_inputs.find(ct)->second;
	  }

      foreach ( unsigned tg, targets_o )
      {
          control_target_names += circ.lines_to_inputs.find(tg)->second; 
//          control_target_names += boost::lexical_cast<std::string>(tg); 
//        control_target_names += _inputs[tg];
      }

      os << cmd << " " << boost::algorithm::join( control_target_names, "," );

      boost::optional<const std::map<std::string, std::string>&> annotations = circ.annotations( g );
      if ( annotations )
      {
        std::string sannotations;
        typedef std::pair<std::string, std::string> pair_t;
        foreach ( const pair_t& p, *annotations )
        {
          sannotations += boost::str( boost::format( " %s=\"%s\"" ) % p.first % p.second );
        }
        os << " #@" << sannotations;
      }

      os << std::endl;
    }

//    os << ".end" << std::endl;
  }

  bool write_realization( const circuit& circ, const std::string& filename, const write_realization_settings& settings, std::string* error )
  {
    std::filebuf fb;
    if ( !fb.open( filename.c_str(), std::ios::out ) )
    {
      if ( error )
      {
        *error = "Cannot open " + filename;
      }
      return false;
    }

    std::ostream os( &fb );

    write_realization( circ, os, settings );

    fb.close();

    return true;
  }

}
