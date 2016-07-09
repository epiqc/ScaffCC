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
 * @file write_specification.hpp
 *
 * @brief Writes a truth table to a RevLib specification file
 */
#ifndef WRITE_SPECIFICATION_HPP
#define WRITE_SPECIFICATION_HPP

#include <string>

#include <core/truth_table.hpp>

namespace revkit
{

  /**
   * @brief Settings for write_specification
   *
   * @author RevKit
   * @since  1.0
   */
  struct write_specification_settings
  {
    /**
     * @brief Default constructor
     *
     * Initializes default values
     *
     * @author RevKit
     * @since  1.0
     */
    write_specification_settings();

    /**
     * @brief A version string
     * 
     * Default value is 2.0 and printed after \b .version command
     *
     * @author RevKit
     * @since  1.0
     */
    std::string version;
    
    /**
     * @brief A header for the file
     *
     * This header will be printed as a comment in the first
     * lines of the file. The string can be multi-line seperated
     * by \\n escape sequences. The comment character # can be
     * omitted and will be inserted automatically.
     *
     * @section Example
     * The following code creates a header in beginning of the
     * file with author information.
     *
     * @code
     * binary_truth_table spec;
     *
     * write_specification_settings settings;
     * settings.header = "Author: Test User\n(c) University";
     * write_specification( spec, "circuit.real", settings );
     * @endcode
     *
     * @author RevKit
     * @since  1.0
     */
    std::string header;

    /**
     * @brief Order of literals in the output cubes
     *
     * The order of the literals in the output cube can
     * be changed with this vector. If not empty, it has to
     * have the same size as the number of outputs in the specification.
     * 
     * It contains of distinct numbers from \em 0 to <i>n-1</i>, where \em n
     * is the size of the input cubes. The numbers assign the literal
     * in the output cube to that index. Empty indices (if the output cubes
     * are smaller than the input cubes) are filled with don't care
     * values.
     *
     * Default value is an empty vector.
     * 
     * @author RevKit
     * @since  1.0
     */
    std::vector<unsigned> output_order;
  };

  /**
   * @brief Writes a truth table to a RevLib specification file
   *
   * @param spec Specification
   * @param filename File-name to write the specification to
   * @param settings Settings
   * @return true on success, false otherwise
   */
  bool write_specification( const binary_truth_table& spec, const std::string& filename, const write_specification_settings& settings = write_specification_settings() );

}

#endif /* WRITE_SPECIFICATION_HPP */
