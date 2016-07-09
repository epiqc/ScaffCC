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

/**
 * @file fully_specified.hpp
 *
 * @brief Determines whether a truth_table is fully specified
 */

#ifndef FULLY_SPECIFIED_HPP
#define FULLY_SPECIFIED_HPP

#include <boost/foreach.hpp>

#include <core/truth_table.hpp>

/** @cond */
#define foreach BOOST_FOREACH
/** @endcond */

namespace revkit
{

  /**
   * @brief Returns whether a truth table is fully specified
   *
   * This function checks whether a truth table \p tt is full specified,
   * that is input dimension is equal to output dimension and there is
   * no don't care value \p dc_value in any of the assigments.
   *
   * Further, the truth table cannot be empty.
   *
   * @note A fully specified truth table does not imply that it is reversible
   *
   * @param tt       Truth table
   * @param dc_value The don't care value for the truth table
   * @param is_reversible If true, number of outputs has to match number of inputs. Default value is \b true.
   * @return true, if it is fully speciefied, otherwise false
   *
   * @author RevKit
   * @since  1.0
   */
  template<typename T>
  bool fully_specified( const truth_table<T>& tt, const typename truth_table<T>::value_type& dc_value, bool is_reversible = true )
  {
    if ( tt.num_inputs() == 0 )
    {
      return false;
    }

    if ( is_reversible && tt.num_inputs() != tt.num_outputs() )
    {
      return false;
    }

    if ( ( 1u << tt.num_inputs() ) != (unsigned)std::distance( tt.begin(), tt.end() ) )
    {
      return false;
    }

    for ( typename truth_table<T>::const_iterator it = tt.begin(); it != tt.end(); ++it )
    {
      if ( std::find( it->first.first, it->first.second, dc_value ) != it->first.second )
      {
        return false;
      }

      if ( std::find( it->second.first, it->second.second, dc_value ) != it->second.second )
      {
        return false;
      }
    }
    
    return true;
  }

  /**
   * @brief Returns whether a truth table is fully specified
   *
   * This function is specialized for a reversible_truth_table,
   * but calls the generic version with the don't care value.
   *
   * @param tt       Truth table
   * @param is_reversible If true, number of outputs has to match number of inputs. Default value is \b true.
   * @return true, if it is fully speciefied, otherwise false
   *
   * @author RevKit
   * @since  1.0
   */
  bool fully_specified( const binary_truth_table& tt, bool is_reversible = true );

}

#endif /* FULLY_SPECIFIED_HPP */
