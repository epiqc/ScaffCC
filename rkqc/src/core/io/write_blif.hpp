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
 * @file write_blif.hpp
 *
 * @brief Writes a circuit to a BLIF file
 */

#ifndef WRITE_BLIF_HPP
#define WRITE_BLIF_HPP

#include <iostream>
#include <map>
#include <vector>

#include <boost/optional.hpp>

namespace revkit
{

  class circuit;
  class gate;

  /**
   * @brief Settings for write_blif
   *
   * @author RevKit
   * @since  1.0
   */
  struct write_blif_settings
  {
    /**
     * @brief Stores truth tables
     *
     * The key is the index of the respective target line.
     * The value is a map itself, which maps input monoms
     * to output values for that target.
     *
     * @author RevKit
     * @since  1.1.1
     */
    typedef std::map<unsigned, std::map<std::vector<boost::optional<bool> >, bool> > truth_table_map;

    /**
     * @brief Standard constructor
     *
     * Initializes default values
     *
     * @author RevKit
     * @since  1.0
     */
    write_blif_settings();

    /**
     * @brief Prefix for the auxiliary variables which are created by the algorithm
     *
     * Default value is \b tmp
     *
     * @author RevKit
     * @since  1.0
     */
    std::string tmp_signal_name;

    /**
     * @brief Sets if output should comply to BlifMV
     *
     * In BlifMV spaces are inserted between the elements
     * of the input cubes.
     *
     * Default value is \b false
     *
     * @author RevKit
     * @since  1.2
     */
    bool blif_mv;

    /**
     * @brief Sets an state prefix for output signals
     *
     * Since states have both the same input and output name
     * this prefix is prepended to the respective output names.
     *
     * Default value is \b out_
     *
     * @author RevKit
     * @since  1.3
     */
    std::string state_prefix;

    /**
     * @brief Sets if constant signals should keep their name
     *
     * Keeps constant names, but only if they are unique.
     *
     * Default value is \b false
     *
     * @author RevKit
     * @since  1.3
     */
    bool keep_constant_names;

    /**
     * @brief Operator for transforming the gates into BLIF code
     *
     * By convention the first input signals are for the target lines and then the control lines.
     * The number of output signals is the number of target lines.
     * Only the cubes have to be printed, not the \em .names declaration.
     *
     * This operator has to be overridden when new gate types should be supported.
     *
     * The signature of this operator changed in RevKit version 1.1.1
     *
     * @param g The current g to be transformed
     * @param map Truth table map to be filled with cubes (check the source code for examples) (since 1.1.1)
     *
     * @author RevKit
     * @since  1.0
     */
    virtual void operator()( const gate& g, truth_table_map& map ) const;

  };

  /**
   * @brief Writes a circuit to a BLIF file
   *
   * This function writes the circuit to a BLIF file.
   * Therefore, it is required that the circuit has names for all its inputs and outputs.
   *
   * @param circ Circuit
   * @param os Output stream where to write the result to. Default is \b STDOUT.
   * @param settings Settings
   *
   * @author RevKit
   * @since  1.0
   */
  void write_blif( const circuit& circ, std::ostream& os = std::cout, const write_blif_settings& settings = write_blif_settings() );

}

#endif /* WRITE_BLIF_HPP */
