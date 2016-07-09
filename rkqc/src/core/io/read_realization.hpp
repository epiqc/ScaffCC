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
 * @file read_realization.hpp
 *
 * @brief Parser for RevLib realization (*.real) file format
 *
 * @author RevKit
 * @since  1.0
 */

#ifndef READ_REALIZATION_HPP
#define READ_REALIZATION_HPP

#include <iosfwd>
#include <vector>

#include <core/circuit.hpp>

#include <core/io/revlib_processor.hpp>

/**
 * @brief Main namespace
 */
namespace revkit
{

  /**
   * @brief Implementation of revlib_processor to construct a circuit
   *
   * This class inherits from revlib_processor and constructs
   * a circuit when parsing a realization file.
   *
   * For convinience the function read_realization(circuit&, const std::string&, std::string*)
   * wraps the use of this class to read circuits from a file.
   *
   * @author RevKit
   * @since  1.0
   */
  class circuit_processor : public revlib_processor
  {
  public:
    /**
     * @brief Default constructor
     *
     * Initializes private data
     *
     * @param circ An empty circuit which will be constructed
     *             and filled with gates while parsing the
     *             circuit
     *
     * @author RevKit
     * @since  1.0
     */
    explicit circuit_processor( circuit& circ );


    /**
     * @brief Default deconstructor
     *
     * Clears private data
     *
     * @author RevKit
     * @since  1.0
     */
    virtual ~circuit_processor();

  protected:
    virtual void on_comment( const std::string& comment ) const;
    virtual void on_numvars( unsigned numvars ) const;
    virtual void on_inputs( std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last ) const;
    virtual void on_outputs( std::vector<std::string>::const_iterator first, std::vector<std::string>::const_iterator last ) const;
    virtual void on_constants( std::vector<constant>::const_iterator first, std::vector<constant>::const_iterator last ) const;
    virtual void on_garbage( std::vector<bool>::const_iterator first, std::vector<bool>::const_iterator last ) const;
    virtual void on_inputbus( const std::string& name, const std::vector<unsigned>& line_indices ) const; 
    virtual void on_outputbus( const std::string& name, const std::vector<unsigned>& line_indices ) const; 
    virtual void on_state( const std::string& name, const std::vector<unsigned>& line_indices, unsigned initial_value ) const;
    virtual void on_module( const std::string& name, const boost::optional<std::string>& filename ) const;
    virtual void on_gate( const boost::any& target_type, const std::vector<unsigned>& line_indices ) const;
    virtual void on_end() const;

  private:
    class priv;
    priv* const d;
  };

  /**
   * @brief Read a circuit realization into a circuit from stream
   *
   * This method uses revlib_parser(std::istream&, revlib_processor&, std::string*)
   * but with a circuit_processor as reader. The
   * required empty circuit for the reader is given as first
   * parameter.
   *
   * @param circ  circuit to be constructed
   * @param in    input stream containing the realization
   * @param error A pointer to a string. In case the parsing fails,
   *              and \p error is not null, a error message is stored
   * @return true on success, false otherwise
   *
   * @author RevKit
   * @since  1.0
   */
  bool read_realization( circuit& circ, std::istream& in, std::string* error = 0 );

  /**
   * @brief Read a circuit realization into a circuit from filename
   *
   * This method construts a \b std::ifstream of the given \p filename
   * and calls read_realization(circuit&, std::istream&, std::string*)
   * with it.
   *
   * @section Example
   *
   * The following code demonstrates how to read a realization from
   * a file given by its filename into a circuit.
   *
   * @code
   * circuit circ;
   * read_realization( circ, "circuit.real" );
   * @endcode
   *
   * Sometimes it is useful to provide the caller with error information
   * in case the call failed. To obtain an error message a pointer to
   * a \b std::string is given as third parameter to the function. The function
   * returns \b false if it fails.
   *
   * @code
   * circuit circ;
   * std::string error;
   *
   * if ( !read_realization( circ, "circuit.real", &error ) ) {
   *   std::cerr << "An error occured: " << error << std::endl;
   * }
   * @endcode
   * 
   * @param circ     circuit to be constructed
   * @param filename filename of the realization
   * @param error    A pointer to a string. In case the parsing fails,
   *                 and \p error is not null, a error message is stored
   * @return true on success, false otherwise
   *
   * @author RevKit
   * @since  1.0
   */
  bool read_realization( circuit& circ, const std::string& filename, std::string* error = 0 );
}

#endif /* READ_REALIZATION_HPP */
