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
 * @file read_pla.hpp
 *
 * @brief Reads a specification from a PLA file
 */

#ifndef READ_PLA_HPP
#define READ_PLA_HPP

#include <core/truth_table.hpp>

namespace revkit
{

  /**
   * @brief Settings for read_pla function
   *
   * @author RevKit
   * @since  1.0
   */
  struct read_pla_settings
  {
    /**
     * @brief Standard constructor
     *
     * Initializes default values
     *
     * @author RevKit
     * @since  1.0
     */
    read_pla_settings();

    /**
     * @brief If true, the truth table is extended after parsing
     *
     * If this variable is set to true, then 1) all cubes
     * like e.g. -01- 1 get extended, so that four cubes
     * are added, which are 0010 1, 0011 1, 1010 1, and 1011 1.
     * That is, all possible Boolean combinations for the don't
     * care values are assigned. Further some PLA implicitly assume
     * that cubes which are not mentioned return 0 as output values
     * for all specified functions. These cubes are added as well.
     *
     * Default value is \b true.
     * 
     * @author RevKit
     * @since  1.0
     */
    bool extend;
  };

  /**
   * @brief Reads a specification from a PLA file
   *
   * This function parses an PLA file and creates a truth table.
   * Thereby only the specified cubes of the PLA are added as entries including the don't care values.
   * For extending the truth table, i.e. filling the don't cares and specifying the 0-outputs explicitly, call extend_truth_table.
   *
   * @param spec The truth table
   * @param filename File-name to read PLA from
   * @param settings Settings for read_pla
   * @param error If not 0, an error message is assigned when the function returns false
   * @return true on success
   *
   * @author RevKit
   * @since  1.0
   */
  bool read_pla( binary_truth_table& spec, const std::string& filename, const read_pla_settings& settings = read_pla_settings(), std::string* error = 0 );

}

#endif /* READ_PLA_HPP */
