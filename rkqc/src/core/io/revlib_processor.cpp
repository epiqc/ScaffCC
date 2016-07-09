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

#include "revlib_processor.hpp"

#include <iostream>

namespace revkit
{

  ////////////////////////////// class revlib_processor
  class revlib_processor::priv
  {
  public:
    priv()
      : current_annotations( new properties() ) {}

    std::vector<std::string> vars;
    properties::ptr current_annotations;
  };

  revlib_processor::revlib_processor()
    : d( new priv() )
  {
  }

  revlib_processor::~revlib_processor()
  {
    delete d;
  }

  void revlib_processor::on_comment( const std::string& comment ) const
  {
  }

  void revlib_processor::on_version( const std::string& version ) const
  {
  }

  void revlib_processor::on_numvars( unsigned numvars ) const
  {
  }

  void revlib_processor::on_variables( std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last ) const
  {
    d->vars.clear();
    std::copy( first, last, std::back_inserter( d->vars ) );
  }

  void revlib_processor::on_inputs( std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last ) const
  {
  }

  void revlib_processor::on_outputs( std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last ) const
  {
  }

  void revlib_processor::on_constants( std::vector<constant>::const_iterator first, std::vector<constant>::const_iterator last ) const
  {
  }

  void revlib_processor::on_garbage( std::vector<bool>::const_iterator first, std::vector<bool>::const_iterator last ) const
  {
  }

  void revlib_processor::on_inputbus( const std::string& name, const std::vector<unsigned>& line_indices ) const
  {
  }

  void revlib_processor::on_outputbus( const std::string& name, const std::vector<unsigned>& line_indices ) const
  {
  }

  void revlib_processor::on_state( const std::string& name, const std::vector<unsigned>& line_indices, unsigned initial_value ) const
  {
  }

  void revlib_processor::on_module( const std::string& name, const boost::optional<std::string>& filename ) const
  {
  }

  void revlib_processor::on_begin() const
  {
  }

  void revlib_processor::on_end() const
  {
  }

  void revlib_processor::on_gate( const boost::any& target_type, const std::vector<unsigned>& line_indices ) const
  {
  }

  void revlib_processor::on_truth_table_line( unsigned line_index, const std::vector<boost::optional<bool> >::const_iterator first, const std::vector<boost::optional<bool> >::const_iterator last ) const
  {
  }

  void revlib_processor::add_annotation( const std::string& key, const std::string& value )
  {
    d->current_annotations->set( key, value );
  }

  void revlib_processor::clear_annotations()
  {
    d->current_annotations->clear();
  }

  properties::ptr revlib_processor::current_annotations() const
  {
    return d->current_annotations;
  }

}
