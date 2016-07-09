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
 * @file truth_table.hpp
 *
 * @brief Class for truth table representation
 */

#ifndef TRUTH_TABLE_HPP
#define TRUTH_TABLE_HPP

#include <iostream>
#include <iterator>
#include <vector>

#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/permutation_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/optional.hpp>

#include <core/circuit.hpp>

namespace revkit
{

  /** @cond */
  template<typename T>
  struct transform_cube;
  /** @endcond */

  /**
   * @brief Represents a  truth table
   *
   * This class helps mapping input assignments
   * to their corresponding output assignments.
   *
   * Assignments are thereby cubes (type truth_table<T>::cube_type)
   * are vectors of values T, which type is given as
   * template parameter to the class.
   *
   * For the tristate value 1, 0, and don't care the
   * type \ref binary_truth_table is predefined with
   * T = boost::optional<bool>.
   *
   * You can use read_specification(binary_truth_table&, const std::string&, std::string*)
   * for reading a RevLib specification file into a truth_table.
   *
   * @section sec_example_iterate_through_truth_table Example
   * This example shows how to iterate through the values of a \ref binary_truth_table, which is not that convenient on the first sight.
   * This code works also for a generic \ref truth_table.
   * @code
   * binary_truth_table tt = // obtained from somewhere
   *
   * for ( binary_truth_table::const_iterator it = tt.begin(); it != tt.end(); ++it )
   * {
   *   // iterate through input cube (bit by bit)
   *   foreach ( const binary_truth_table::value_type& in_bit, it->first )
   *   {
   *     // do something with in_bit
   *   }
   *
   *   // iterate through output cube (bit by bit)
   *   foreach ( const binary_truth_table::value_type& out_bit, it->second )
   *   {
   *     // do something with out_bit
   *   }
   * }
   * @endcode
   *
   * @author RevKit
   * @since  1.0
   */
  template<typename T>
  class truth_table
  {
  public:
    /**
     * @brief Typedef reference to the given template type
     *
     * @author RevKit
     * @since  1.0
     */
    typedef T                                     value_type;

    /**
     * @brief Type representing a cube
     *
     * Implemented as a vector over the basic type T
     *
     * @author RevKit
     * @since  1.0
     */
    typedef std::vector<T>                        cube_type;

    /**
     * @brief Represents a map from input to output cube
     *
     * Implemented as a tuple
     *
     * @author RevKit
     * @since  1.0
     */
    typedef std::map<cube_type, cube_type> cube_vector;

    /**
     * @brief Constant Iterator of input cubes
     *
     * Default constant iterator is used.
     *
     * @author RevKit
     * @since  1.0
     */
    typedef typename cube_type::const_iterator in_const_iterator;

    /**
     * @brief Constant Iterator of output cubes
     *
     * A permutation iterator from Boost.Iterators is used which makes use of the truth table's permutation.
     *
     * @author RevKit
     * @since  1.0
     */
    typedef boost::permutation_iterator<typename cube_type::const_iterator, std::vector<unsigned>::const_iterator> out_const_iterator;

    /**
     * @brief Truth Table's constant iterator
     *
     * A transform iterator which transforms the cube_tuple objects to a pair of iterator pairs of each input and output cube.
     *
     * @author RevKit
     * @since  1.0
     */
    typedef boost::transform_iterator<transform_cube<T>, typename cube_vector::const_iterator> const_iterator;

    /**
     * @brief Returns the number of inputs
     *
     * If the truth table contains no cube tuple,
     * then 0 is returned, otherwise the length of the
     * first input assignment is returned.
     *
     * @return Number of inputs
     *
     * @author RevKit
     * @since  1.0
     */
    unsigned num_inputs() const
    {
      if ( _cubes.size() )
      {
        return _cubes.begin()->first.size();
      }
      else
      {
        return 0;
      }
    }

    /**
     * @brief Returns the number of outputs
     *
     * If the truth table contains no cube tuple,
     * then 0 is returned, otherwise the length of the
     * first output assignment is returned.
     *
     * @return Number of outputs
     *
     * @author RevKit
     * @since  1.0
     */
    unsigned num_outputs() const
    {
      if ( _cubes.size() )
      {
        return _cubes.begin()->second.size();
      }
      else
      {
        return 0;
      }
    }

    /**
     * @brief Returns constant begin iterator of the cube list
     *
     * @return Constant begin iterator of the cube list
     *
     * @author RevKit
     * @since  1.0
     */
    const_iterator begin() const
    {
      return boost::make_transform_iterator( _cubes.begin(), transform_cube<T>( _permutation ) );
    }

    /**
     * @brief Returns constant end iterator of the cube list
     *
     * @return Constant end iterator of the cube list
     *
     * @author RevKit
     * @since  1.0
     */
    const_iterator end() const
    {
      return boost::make_transform_iterator( _cubes.end(), transform_cube<T>( _permutation ) );
    }

    /**
     * @brief Adds a new entry to the truth table
     *
     * With adding the first entry the dimension of inputs
     * and outputs is set. When adding further entries
     * it has to make sure that the dimensions fit, else
     * an assertion is thrown and false is returned.
     *
     * @param input Input assignment
     * @param output Output assignment
     * @return Returns whether the assignment could be added or not
     *
     * @author RevKit
     * @since  1.0
     */
    bool add_entry( const cube_type& input, const cube_type& output )
    {
      if ( _cubes.size() &&
           ( input.size() != _cubes.begin()->first.size() ||
             output.size() != _cubes.begin()->second.size() ) )
      {
        assert( false );
        return false;
      }

      if ( !_cubes.size() )
      {
        /* first entry -> create permutation */
        std::copy( boost::counting_iterator<unsigned>( 0 ),
                   boost::counting_iterator<unsigned>( output.size() ),
                   std::back_inserter( _permutation ) );

        _constants.resize( input.size(), constant() );
        _garbage.resize( output.size(), false );
      }

      _cubes.insert( std::make_pair( input, output ) );
      return true;
    }

    /**
     * @brief Clears the truth table
     *
     * Clears the truth table, as well as the current permutation and constant
     * and garbage information.
     *
     * @author RevKit
     * @since  1.0
     */
    void clear()
    {
      _cubes.clear();
      _permutation.clear();
      _constants.clear();
      _garbage.clear();
    }

    /**
     * @brief Returns current permutation
     *
     * The permutation is initializes when the first entry is added
     * to the truth table and is initially the sequence from 0 to \e n - 1,
     * where \e n is the size of the output cubes.
     *
     * @return Current permutation
     *
     * @author RevKit
     * @since  1.0
     */
    const std::vector<unsigned>& permutation() const
    {
      return _permutation;
    }

    /**
     * @brief Sets the permutation
     *
     * This method can set a specific permutation. This method should not be used
     * in combination with permute which provides a dynamic change of the permutation.
     *
     * @param perm New permutation
     * @return True, if successful. It can be unsuccessful, when the size of perm is not suitable.
     *
     * @author RevKit
     * @since  1.0
     */
    bool set_permutation( const std::vector<unsigned>& perm )
    {
      if ( perm.size() == _permutation.size() )
      {
        std::copy( perm.begin(), perm.end(), _permutation.begin() );
        return true;
      }
      else
      {
        return false;
      }
    }

    /**
     * @brief Permutes the current permutation
     *
     * This methods calls <tt>std::next_permutation</tt> on the current permutation.
     * It returns false, when all permutations were considered.
     *
     * @return False, when all permutations were considered, true otherwise.
     *
     * @author RevKit
     * @since  1.0
     */
    bool permute()
    {
      return std::next_permutation( _permutation.begin(), _permutation.end() );
    }

    /**
     * @brief Sets the inputs of the specification
     *
     * Use \ref copy_metadata to assign specification meta-data to a circuit.
     *
     * @param ins Vector of input names
     *
     * @author RevKit
     * @since  1.0
     */
    void set_inputs( const std::vector<std::string>& ins )
    {
      _inputs = ins;
    }

    /**
     * @brief Returns the inputs of the specification
     *
     * Use \ref copy_metadata to assign specification meta-data to a circuit.
     *
     * @return Vector of input names
     *
     * @author RevKit
     * @since  1.0
     */
    const std::vector<std::string>& inputs() const
    {
      return _inputs;
    }

    /**
     * @brief Sets the outputs of the specification
     *
     * Use \ref copy_metadata to assign specification meta-data to a circuit.
     *
     * @param outs Vector of output names
     *
     * @author RevKit
     * @since  1.0
     */
    void set_outputs( const std::vector<std::string>& outs )
    {
      _outputs = outs;
    }

    /**
     * @brief Returns the outputs of the specification
     *
     * The outputs are permuted in respect to the current permutation.
     *
     * Use \ref copy_metadata to assign specification meta-data to a circuit.
     *
     * @return Vector of output names
     *
     * @author RevKit
     * @since  1.0
     */
    std::vector<std::string> outputs() const
    {
      if ( _outputs.size() == _permutation.size() )
      {
        // permute outputs first
        return std::vector<std::string>( boost::make_permutation_iterator( _outputs.begin(), _permutation.begin() ), boost::make_permutation_iterator( _outputs.begin(), _permutation.end() ) );
      }
      else
      {
        return _outputs;
      }
    }

    /**
     * @brief Sets the constant lines of the specification
     *
     * Use \ref copy_metadata to assign specification meta-data to a circuit.
     *
     * @param constants Vector of constant values
     *
     * @author RevKit
     * @since  1.0
     */
    void set_constants( const std::vector<constant>& constants )
    {
      _constants = constants;
      _constants.resize( num_inputs(), constant() );
    }

    /**
     * @brief Returns the constant line information of the specification
     *
     * Use \ref copy_metadata to assign specification meta-data to a circuit.
     *
     * @return Vector of constant line information
     *
     * @author RevKit
     * @since  1.0
     */
    const std::vector<constant>& constants() const
    {
      return _constants;
    }

    /**
     * @brief Sets the garbage lines of the specification
     *
     * Use \ref copy_metadata to assign specification meta-data to a circuit.
     *
     * @param garbage Vector of garbage values
     *
     * @author RevKit
     * @since  1.0
     */
    void set_garbage( const std::vector<bool>& garbage )
    {
      _garbage = garbage;
      _garbage.resize( num_outputs(), false );
    }

    /**
     * @brief Returns the garbage line information of the specification
     *
     * The garbage line information is permuted in respect to the current permutation.
     *
     * Use \ref copy_metadata to assign specification meta-data to a circuit.
     *
     * @return Vector of garbage line information
     *
     * @author RevKit
     * @since  1.0
     */
    std::vector<bool> garbage() const
    {
      if ( _garbage.size() == _permutation.size() )
      {
        // permute outputs first
        return std::vector<bool>( boost::make_permutation_iterator( _garbage.begin(), _permutation.begin() ), boost::make_permutation_iterator( _garbage.begin(), _permutation.end() ) );
      }
      else
      {
        return _garbage;
      }
    }

  private:
    /** @cond */
    cube_vector _cubes;
    std::vector<unsigned> _permutation;
    std::vector<std::string> _inputs;
    std::vector<std::string> _outputs;
    std::vector<constant> _constants;
    std::vector<bool> _garbage;
    /** @endcond */
  };

  /** @cond */
  template<typename T>
  struct transform_cube
  {
    explicit transform_cube( const std::vector<unsigned>& permutation ) : permutation( permutation ) {}

    typedef std::pair<typename truth_table<T>::in_const_iterator, typename truth_table<T>::in_const_iterator> in_const_iterator_pair;
    typedef std::pair<typename truth_table<T>::out_const_iterator, typename truth_table<T>::out_const_iterator> out_const_iterator_pair;
    typedef std::pair<in_const_iterator_pair, out_const_iterator_pair> result_type;

    result_type operator()( const typename truth_table<T>::cube_vector::value_type& ct ) const
    {
      return std::make_pair(
               std::make_pair( ct.first.begin(), ct.first.end() ),
               std::make_pair(
                 boost::make_permutation_iterator( ct.second.begin(), permutation.begin() ),
                 boost::make_permutation_iterator( ct.second.end(), permutation.end() )
               )
             );
    }

  private:
    const std::vector<unsigned>& permutation;
  };
  /** @endcond */

  /**
   * @brief A predefined truth table for specifications using binary values as in specifications for reversible circuits
   *
   * As template type boost::optional<bool> is used, which
   * represents 0, 1, and a don't care value.
   *
   * * <table border="0">
     *   <tr>
     *     <td class="indexkey">Description</th>
     *     <td class="indexkey">Char representation</th>
     *     <td class="indexkey">Typed value</th>
     *   </tr>
     *   <tr>
     *     <td class="indexvalue">No constant input line</td>
     *     <td align="center" class="indexvalue">'-'</td>
     *     <td class="indexvalue">@code boost::optional<bool>() @endcode</td>
     *   </tr>
     *   <tr>
     *     <td class="indexvalue">Constant input line with value 0</td>
     *     <td align="center" class="indexvalue">'0'</td>
     *     <td class="indexvalue">@code boost::optional<bool>( 0 ) @endcode</td>
     *   </tr>
     *   <tr>
     *     <td class="indexvalue">Constant input line with value 1</td>
     *     <td align="center" class="indexvalue">'1'</td>
     *     <td class="indexvalue">@code boost::optional<bool>( 1 ) @endcode</td>
     *   </tr>
     * </table>
   */
  typedef truth_table<boost::optional<bool> > binary_truth_table;

  /**
   * @brief Outputs a truth table
   *
   * Prints the input and output cubes of a binary truth table
   *
   * @param os The output stream
   * @param spec The truth table
   * @return The output stream \p os
   *
   * @author RevKit
   * @since  1.0
   */
  std::ostream& operator<<( std::ostream& os, const binary_truth_table& spec );

  /**
   * @brief Converts a truth table cube to a number
   *
   * The first element in the cube (at index 0) is thereby the most significant bit.
   *
   * @param cube The cube to be converted
   * @return The \p cube in numerical representation
   *
   * @author RevKit
   * @since  1.0
   */
  unsigned truth_table_cube_to_number( const binary_truth_table::cube_type& cube );

  /**
   * @brief Converts a number to a cube of a fixed bitwidth
   *
   * The first element in the cube (at index 0) is thereby the most significant bit.
   *
   * @param number Number to be converted as a cube
   * @param bw Bit-width of the cube
   * @return The number as cube
   *
   * @author RevKit
   * @since  1.0
   */
  binary_truth_table::cube_type number_to_truth_table_cube( unsigned number, unsigned bw );

}

#endif /* TRUTH_TABLE_HPP */
