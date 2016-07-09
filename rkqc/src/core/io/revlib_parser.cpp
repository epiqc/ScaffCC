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

#include "revlib_parser.hpp"

#include <iostream>
#include <locale>
#include <map>
#include <stack>

#include <boost/algorithm/string/find.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/spirit/include/qi.hpp>

#include "../target_tags.hpp"
#include "revlib_processor.hpp"

using namespace boost::assign;

namespace revkit
{

  struct transform_to_garbage
  {
    bool operator()( const char& c ) const
    {
      return c == '1';
    }
  };

  struct transform_to_constants
  {
    constant operator()( const char& c ) const
    {
      switch ( c ) {
      case '-':
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

  // CODE move to utils
  template<typename Map>
  struct map_access_functor
  {
    explicit map_access_functor( Map& map )
      : _map( map )
    {
    }

    const typename Map::mapped_type& operator()( const typename Map::key_type& key ) const
    {
      return _map.find( key )->second;
    }

  private:
    Map& _map;
  };

  template<typename Map>
  map_access_functor<Map> make_map_access_functor( Map& map )
  {
    return map_access_functor<Map>( map );
  }

  bool parse_string_list( const std::string& line, std::vector<std::string>& params )
  {
    namespace ascii = boost::spirit::ascii;
    namespace qi = boost::spirit::qi;

    std::string::const_iterator it = line.begin();
    bool r = qi::parse( it, line.end(),
                        *(
                          (   qi::lexeme['"' >> +( qi::char_ - '"' ) >> '"'] // string with quotes
                            | +( qi::char_ - ' ' ) )                         // without quotes
                          >> -qi::lit( ' ' )
                         ), params );

    return ( r && it == line.end() );
  }

  bool parse_annotations( const std::string& line, std::vector<boost::fusion::vector<std::string, std::string> >& annotations )
  {
    namespace ascii = boost::spirit::ascii;
    namespace qi = boost::spirit::qi;

    std::string::const_iterator it = line.begin();
    bool r = qi::parse( it, line.end(),
                        *( qi::lexeme[+( ascii::alnum | '-' )] >> '=' >>
                          (   qi::lexeme['"' >> +( qi::char_ - '"' ) >> '"'] // string with quotes
                            | +( qi::char_ - ' ' ) )                         // without quotes
                           >> *qi::lit( ' ' )
                         ), annotations );

    return ( r && it == line.end() );
  }

  bool is_number( const std::string& str )
  {
    foreach ( const char& c, str )
    {
      if ( !std::isdigit( c ) )
      {
        return false;
      }
    }

    return true;
  }


  bool revlib_parser( std::istream& in, revlib_processor& reader, const std::string& base_directory, std::string* error )
  {
    std::string line;

    unsigned numvars = 0;
    unsigned truth_table_index = 0;
    std::stack<std::map<std::string, unsigned> > variable_indices;
    std::vector<std::string> module_names;

    while ( in.good() && getline( in, line ) )
    {
      /* clear previous annotations */
      reader.clear_annotations();

      /* extract comments */
      if ( boost::iterator_range<std::string::iterator> result = boost::algorithm::find_first( line, "#" ) )
      {
        std::string comment( result.begin() + 1u, line.end() );

        if ( !comment.empty() && comment.at( 0u ) == '@' )
        {
          std::string sannotations( comment.begin() + 1u, comment.end() );
          std::vector<boost::fusion::vector<std::string, std::string> > annotations;
          boost::algorithm::trim( sannotations );
          parse_annotations( sannotations, annotations );

          typedef boost::fusion::vector<std::string, std::string> pair_t;
          foreach ( const pair_t& pair, annotations )
          {
            reader.add_annotation( boost::fusion::at_c<0>( pair ), boost::fusion::at_c<1>( pair ) );
          }
        }
        else
        {
          reader.on_comment( comment );
        }

        line.erase( result.begin(), line.end() );
      }

      boost::algorithm::trim( line );

      /* skip empty lines */
      if ( !line.size() )
      {
        continue;
      }

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

      if ( command == "#" )
      {
        /* All parameters combined are considered as comment */
        reader.on_comment( boost::algorithm::join( params, " " ) );
      }
      else if ( command == ".version" )
      {
        /* All parameters combined are considered as version */
        reader.on_version( boost::algorithm::join( params, " " ) );
      }
      else if ( command == ".numvars" )
      {
        if ( params.size() != 1 )
        {
          if ( error )
          {
            *error = "Invalid number of parameters for .numvars command";
          }
          return false;
        }

        try
        {
          numvars = boost::lexical_cast<unsigned>( params.front() );
          reader.on_numvars( numvars );
        }
        catch ( boost::bad_lexical_cast& )
        {
          if ( error )
          {
            *error = "Invalid parameter for .numvars command";
          }
          return false;
        }
      }
      else if ( command == ".variables" )
      {
        if ( params.size() != numvars )
        {
          if ( error )
          {
            *error = "Variable count does not fit numvars";
          }
          return false;
        }

        /* fill the index map later used when processing the gates */
        std::map<std::string, unsigned> map;
        for ( std::vector<std::string>::const_iterator it = params.begin(); it != params.end(); ++it )
        {
          map.insert( std::make_pair( *it, it - params.begin() ) );
        }
        variable_indices.push( map );

        reader.on_variables( params.begin(), params.end() );
      }
      else if ( command == ".inputs" )
      {
        // since the inputs can contain spaces in quotes we have to deal with
        // them specifically
        std::string inputs_str( line.begin() + command.size() + 1u, line.end() );
        boost::algorithm::trim( inputs_str );
        std::vector<std::string> inputs;

        if ( !parse_string_list( inputs_str, inputs ) )
        {
          if ( error )
          {
            *error = "Cannot parse .input command";
          }
          return false;
        }

        if ( inputs.size() != numvars )
        {
          if ( error )
          {
            *error = "Input count does not fit numvars";
          }
          return false;
        }

        reader.on_inputs( inputs.begin(), inputs.end() );
      }
      else if ( command == ".outputs" )
      {
        // see .inputs
        std::string outputs_str( line.begin() + command.size() + 1u, line.end() );
        boost::algorithm::trim( outputs_str );
        std::vector<std::string> outputs;

        if ( !parse_string_list( outputs_str, outputs ) )
        {
          if ( error )
          {
            *error = "Cannot parse .output command";
          }
          return false;
        }

        if ( outputs.size() != numvars )
        {
          if ( error )
          {
            *error = "Output count does not fit numvars";
          }
          return false;
        }

        reader.on_outputs( outputs.begin(), outputs.end() );
      }
      else if ( command == ".constants" )
      {
        if ( params.size() != 1 || params.front().size() != numvars )
        {
          if ( error )
          {
            *error = "Constant count does not fit numvars";
          }
          return false;
        }

        std::vector<constant> constants( numvars );
        std::transform( params.front().begin(), params.front().end(), constants.begin(), transform_to_constants() );

        reader.on_constants( constants.begin(), constants.end() );
      }
      else if ( command == ".garbage" )
      {
        if ( params.size() != 1 || params.front().size() != numvars )
        {
          if ( error )
          {
            *error = "Garbage count does not fit numvars";
          }
          return false;
        }

        std::vector<bool> garbage( numvars );
        std::transform( params.front().begin(), params.front().end(), garbage.begin(), transform_to_garbage() );

        reader.on_garbage( garbage.begin(), garbage.end() );
      }
      else if ( command == ".inputbus" )
      {
        if ( params.size() < 2u )
        {
          if ( error )
          {
            *error = "Too few arguments in .inputbus command";
          }
          return false;
        }

        std::vector<unsigned> line_indices( params.size() - 1u );
        std::transform( params.begin() + 1u, params.end(), line_indices.begin(), make_map_access_functor( variable_indices.top() ) );

        reader.on_inputbus( params.front(), line_indices );
      }
      else if ( command == ".outputbus" )
      {
        if ( params.size() < 2u )
        {
          if ( error )
          {
            *error = "Too few arguments in .outputbus command";
          }
          return false;
        }

        std::vector<unsigned> line_indices( params.size() - 1u );
        std::transform( params.begin() + 1u, params.end(), line_indices.begin(), make_map_access_functor( variable_indices.top() ) );

        reader.on_outputbus( params.front(), line_indices );
      }
      else if ( command == ".state" )
      {
        if ( params.size() < 2u )
        {
          if ( error )
          {
            *error = "Too few arguments in .state command";
          }
          return false;
        }

        boost::optional<unsigned> initial_value;

        if ( is_number( params.back() ) )
        {
          if ( params.size() == 2u )
          {
            if ( error )
            {
              *error = "Too few arguments in .state command";
            }
            return false;
          }

          initial_value = boost::lexical_cast<unsigned>( params.back() );
        }

        unsigned offset = initial_value ? 1u : 0u;

        std::vector<unsigned> line_indices( params.size() - 1u - offset );
        std::transform( params.begin() + 1u, params.end() - offset, line_indices.begin(), make_map_access_functor( variable_indices.top() ) );

        reader.on_state( params.front(), line_indices, initial_value.get_value_or( 0u ) );
      }
      else if ( command == ".module" )
      {
        assert( params.size() <= 2u );
        std::string name = params.front();

        boost::optional<std::string> filename;
        if ( params.size() == 2u )
        {
          filename = base_directory + "/" + params.back();
        }

        if ( filename && !boost::filesystem::exists( *filename ) )
        {
          if ( error )
          {
            *error = boost::str( boost::format( "File for module %s not found" ) % name );
          }
          return false;
        }

        module_names += name;

        reader.on_module( name, filename );
      }
      else if ( command == ".begin" )
      {
        if ( params.size() != 0 )
        {
          if ( error )
          {
            *error = "Wrong number of parameters for .begin command";
          }
          return false;
        }

        reader.on_begin();
      }
      else if ( command == ".end" )
      {
        if ( params.size() != 0 )
        {
          if ( error )
          {
            *error = "Wrong number of parameters for .end command";
          }
          return false;
        }

        if ( !variable_indices.empty() )
        {
          variable_indices.pop();
        }
        reader.on_end();
      }
      else
      {
        if ( command[0] == '-' || command[0] == '1' || command[0] == '0' )
        {
          // truth table line
          if ( params.size() )
          {
            if ( error )
            {
              *error = "Params in truth table line";
            }
            return false;
          }

          std::vector<boost::optional<bool> > cube( command.size() );
          std::transform( command.begin(), command.end(), cube.begin(), transform_to_constants() );

          reader.on_truth_table_line( truth_table_index++, cube.begin(), cube.end() );
        }
        else
        {
          // gate
          std::vector<unsigned> line_indices( params.size() );
          std::transform( params.begin(), params.end(), line_indices.begin(), make_map_access_functor( variable_indices.top() ) );

          boost::any gate_type;

          if ( boost::range::find( module_names, command ) != module_names.end() )
          {
            module_tag module_t;
            module_t.name = command;
            gate_type = module_t;
          }
          else
          {
            switch ( command[0] )
            {
            case 't':
              gate_type = toffoli_tag();
              break;

            case 'p':
              gate_type = peres_tag();
              break;

            case 'f':
              gate_type = fredkin_tag();
              break;

            case 'v':
              if ( command == "v+" )
              {
                gate_type = vplus_tag();
              }
              else if ( command == "v" )
              {
                gate_type = v_tag();
              }
              else
              {
                if ( error )
                {
                  *error = "unknown gate command: " + command;
                }
                return false;
              }
              break;

            default:
              if ( error )
              {
                *error = "unknown gate command: " + command;
              }
            }
          }

          reader.on_gate( gate_type, line_indices );
        }
      }
    }

    return true;
  }

}
