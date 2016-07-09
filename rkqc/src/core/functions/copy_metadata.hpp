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
 * @file copy_metadata.hpp
 *
 * Copy meta-data from specification or circuit to circuit
 */

#ifndef COPY_METADATA_HPP
#define COPY_METADATA_HPP

#include "../circuit.hpp"
#include "../truth_table.hpp"

namespace revkit
{

  /**
   * @brief Settings for copy_metadata
   *
   * With this settings structure some parts can be
   * deactivated for copying. In the default case,
   * all information of the circuit is copied.
   *
   * @author RevKit
   * @since  1.2
   *
   */
  struct copy_metadata_settings
  {
    /**
     * @brief Standard Constructor
     *
     * Initializes default values
     *
     * @author RevKit
     * @since  1.2
     */
    copy_metadata_settings();

    /**
     * @brief Input names are copied
     *
     * The default value is \b true.
     *
     * @author RevKit
     * @since  1.2
     */
    bool copy_inputs;

    /**
     * @brief Output names are copied
     *
     * The default value is \b true.
     *
     * @author RevKit
     * @since  1.2
     */
    bool copy_outputs;

    /**
     * @brief Constant line information is copied
     *
     * The default value is \b true.
     *
     * @author RevKit
     * @since  1.2
     */
    bool copy_constants;

    /**
     * @brief Garbage line information is copied
     *
     * The default value is \b true.
     *
     * @author RevKit
     * @since  1.2
     */
    bool copy_garbage;

    /**
     * @brief Circuit name is copied
     *
     * The default value is \b true.
     *
     * @author RevKit
     * @since  1.2
     */
    bool copy_name;

    /**
     * @brief Input buses are copied
     *
     * The default value is \b true.
     *
     * @author RevKit
     * @since  1.2
     */
    bool copy_inputbuses;

    /**
     * @brief Output buses are copied
     *
     * The default value is \b true.
     *
     * @author RevKit
     * @since  1.2
     */
    bool copy_outputbuses;

    /**
     * @brief State signals are copied
     *
     * The default value is \b true.
     *
     * @author RevKit
     * @since  1.2
     */
    bool copy_statesignals;

    /**
     * @brief Modules are copied
     *
     * The default value is \b true.
     *
     * @author RevKit
     * @since  1.2
     */
    bool copy_modules;
  };

  /**
   * @brief Copies meta-data from a specification to a circuit
   *
   * This method reads the inputs and outputs from a specification
   * and assigns to it to a circuit. Truth-table based synthesis
   * algorithms should use this method.
   *
   * @param spec Truth Table
   * @param circ Circuit
   *
   * @author RevKit
   * @since  1.0
   */
  template<typename T>
  void copy_metadata( const truth_table<T>& spec, circuit& circ )
  {
    circ.set_inputs( spec.inputs() );
    circ.set_outputs( spec.outputs() );
    circ.set_constants( spec.constants() );
    circ.set_garbage( spec.garbage() );
  }

  /**
   * @brief Copies meta-data from a circuit to another circuit
   *
   * This method copies everything but the gates from circuit \p base
   * to circuit \p circ.
   *
   * @param base Source circuit
   * @param circ Destination circuit
   * @param settings Settings for copy_metadata (since 1.2)
   *
   * @author RevKit
   * @since  1.0
   */
  void copy_metadata( const circuit& base, circuit& circ, const copy_metadata_settings& settings = copy_metadata_settings() );

}

#endif /* COPY_METADATA_HPP */
