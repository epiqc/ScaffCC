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

#include "read_pla.hpp"

#include <fstream>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

#include "../functions/extend_truth_table.hpp"

namespace revkit
{

  struct transform_to_constants
  {
    constant operator()( const char& c ) const
    {
      switch ( c ) {
      case '-':
      case '~':
        return constant();
        break;

      case '0':
      case '1':
        return constant( c == '1' );
        break;

      default:
        assert( false );
        return constant();
        break;
      }
    }
  };

  read_pla_settings::read_pla_settings()
    : extend( true )
  {
  }

  bool read_pla( binary_truth_table& spec, std::istream& in, const read_pla_settings& settings, std::string* error )
  {
    std::string line;

    spec.clear();

    while ( in.good() && getline( in, line ) )
    {
      boost::algorithm::trim( line );

      /* skip empty lines */
      if ( !line.size() )
      {
        continue;
      }

      // command?
      if ( line.at( 0 ) == '.' )
      {
        std::vector<std::string> params;
        boost::algorithm::split( params, line, boost::algorithm::is_any_of( " " ) );


        /* It is possible that there are empty elements in params,
           e.g. when line contains two spaces between identifiers instead of one.
           These should be removed. */
        std::vector<std::string>::iterator newEnd = std::remove( params.begin(), params.end(), "" );
        params.erase( newEnd, params.end() );

        /* By means of the first element we can determine the command */
        std::string command = params.front();
        params.erase( params.begin() );

        if ( command == ".i" || command == ".o" || command == ".e" )
        {
          // skip, will be set automatically by input output line
        }
        else if ( command == ".ilb" )
        {
          spec.set_inputs( params );
        }
        else if ( command == ".ob" )
        {
          spec.set_outputs( params );
        }
      }
      else if ( line.at( 0 ) == '#' )
      {
        // skip comments for now
      }
      else
      {
        assert( line.at( 0 ) == '-' || line.at( 0 ) == '1' || line.at( 0 ) == '0' );

        std::vector<std::string> cubes;
        boost::algorithm::split( cubes, line, boost::algorithm::is_any_of( " \t" ) );
        cubes.erase( std::remove( cubes.begin(), cubes.end(), "" ), cubes.end() );

        assert( cubes.size() == 2 );

        std::vector<boost::optional<bool> > cube_in( cubes.at( 0 ).size() );
        std::transform( cubes.at( 0 ).begin(), cubes.at( 0 ).end(), cube_in.begin(), transform_to_constants() );

        std::vector<boost::optional<bool> > cube_out( cubes.at( 1 ).size() );
        std::transform( cubes.at( 1 ).begin(), cubes.at( 1 ).end(), cube_out.begin(), transform_to_constants() );

        spec.add_entry( cube_in, cube_out );
      }
    }

    if ( settings.extend )
    {
      extend_truth_table( spec );
    }

    return true;
  }

  bool read_pla( binary_truth_table& spec, const std::string& filename, const read_pla_settings& settings, std::string* error )
  {
    std::ifstream is;
    is.open( filename.c_str(), std::ifstream::in );

    if ( !is.good() )
    {
      if ( error )
      {
        *error = "Cannot open " + filename;
      }
      return false;
    }

    return read_pla( spec, is, settings, error );
  }

}

