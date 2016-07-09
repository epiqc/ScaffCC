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
 * @file write_verilog.hpp
 *
 * @brief Writes a circuit to a Verilog file
 *
 * @since 1.1
 */

#ifndef WRITE_VERILOG_HPP
#define WRITE_VERILOG_HPP

#include <iostream>

namespace revkit
{

  class circuit;

  /**
   * @brief Settings for write_verilog
   *
   * @author RevKit
   * @since  1.1
   */
  struct write_verilog_settings
  {
    /**
     * @brief Standard constructor
     *
     * Initializes default values
     *
     * @author RevKit
     * @since  1.1
     */
    write_verilog_settings();

    /**
     * @brief Determines whether the constants are propagated
     *
     * If the constants are propagated, then no signals
     * are created for constant inputs. Instead, the values
     * are simulated by creating the Verilog file.
     *
     * The default value is \b true.
     *
     * @author RevKit
     * @since  1.1
     */
    bool propagate_constants;
  };

  /**
   * @brief Writes a circuit to a Verilog file
   *
   * This function dumps the circuit as a Verilog file.
   *
   * @param circ Circuit
   * @param os Output Stream to write to
   * @param settings Settings
   *
   * @author RevKit
   * @since  1.1
   */
  void write_verilog( const circuit& circ, std::ostream& os = std::cout, const write_verilog_settings& settings = write_verilog_settings() );

}

#endif /* WRITE_VERILOG_HPP */
