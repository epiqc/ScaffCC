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

#include "read_specification.hpp"

#include <fstream>
#include <iostream>

#include "revlib_parser.hpp"

namespace revkit
{

  ////////////////////////////// class specification_processor
  class specification_processor::priv
  {
  public:
    priv( binary_truth_table& s )
    : spec( s ),
      has_entries( false ) {}

    binary_truth_table& spec;
    std::vector<constant> constants;
    std::vector<bool> garbage;
    bool has_entries;
  };

  specification_processor::specification_processor( binary_truth_table& spec )
    : revlib_processor(), d( new priv( spec ) )
  {
  }

  specification_processor::~specification_processor()
  {
    delete d;
  }

  void specification_processor::on_inputs( std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last ) const
  {
    std::vector<std::string> inputs( first, last );
    d->spec.set_inputs( inputs );
  }

  void specification_processor::on_outputs( std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last ) const
  {
    std::vector<std::string> outputs( first, last );
    d->spec.set_outputs( outputs );
  }

  void specification_processor::on_constants( std::vector<constant>::const_iterator first, std::vector<constant>::const_iterator last ) const
  {
    d->constants.assign( first, last );
  }

  void specification_processor::on_garbage( std::vector<bool>::const_iterator first, std::vector<bool>::const_iterator last ) const
  {
    d->garbage.assign( first, last );
  }

  
  void specification_processor::on_truth_table_line( unsigned line_index, const std::vector<boost::optional<bool> >::const_iterator first, const std::vector<boost::optional<bool> >::const_iterator last ) const
  {
    unsigned bw = last - first;

    binary_truth_table::cube_type in;
    binary_truth_table::cube_type out( first, last );

    for ( int pos = bw - 1; pos >= 0; --pos )
    {
      in.push_back( binary_truth_table::value_type( line_index & ( 1u << pos ) ) );
    }

    d->spec.add_entry( in, out );

    if ( !d->has_entries )
    {
      d->spec.set_constants( d->constants );
      d->spec.set_garbage( d->garbage );
      d->has_entries = true;
    }
  }

  bool read_specification( binary_truth_table& spec, std::istream& in, std::string* error )
  {
    specification_processor processor( spec );

    return revlib_parser( in, processor, ".", error );
  }

  bool read_specification( binary_truth_table& spec, const std::string& filename, std::string* error )
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

    return read_specification( spec, is, error );
  }

}
