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
 * @file write_realization.hpp
 *
 * @brief Generator for RevLib realization (*.real) format
 *
 * @author RevKit
 * @since  1.0
 */

#ifndef WRITE_REALIZATION_HPP
#define WRITE_REALIZATION_HPP

#include <iosfwd>
#include <string>

#include <core/circuit.hpp>

namespace revkit
{

  /**
   * @brief Settings for write_realization function
   *
   * @author RevKit
   * @since  1.0
   */
  struct write_realization_settings
  {
    /**
     * @brief Default constructor
     *
     * Initializes default values
     *
     * @author RevKit
     * @since  1.0
     */
    write_realization_settings();

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
     * circuit circ;
     *
     * write_realization_settings settings;
     * settings.header = "Author: Test User\n(c) University";
     * write_realization( circ, "circuit.real", settings );
     * @endcode
     *
     * @author RevKit
     * @since  1.0
     */
    std::string header;
  };

  /**
   * @brief Writes a circuit as RevLib realization to an output stream
   *
   * This method takes a circuit and writes it to an output stream.
   * Custom settings can be controlled via the \p settings parameter,
   * which has default values if not set explicitely.
   *
   * It may be more convenient to use the write_realization(const circuit&, const std::string&, const write_realization_settings&, std::string*)
   * function which takes a filename as parameter instead of an output stream.
   *
   * This function can be useful when dumping to e.g. STDOUT.
   *
   * @section Example
   * The following example writes a circuit to STDOUT.
   *
   * @code
   * circuit circ;
   * write_realization( circ, std::cout ); 
   * @endcode
   *
   * @param circ     Circuit to write
   * @param os       Output stream
   * @param settings Settings (see write_realization_settings)
   *
   * @author RevKit
   * @since  1.0
   */
  void write_realization( const circuit& circ, std::ostream& os, const write_realization_settings& settings = write_realization_settings() );

  /**
   * @brief Writes a circuit as RevLib realization to a file
   *
   * This is a wrapper function for write_realization(const circuit&, std::ostream&, const write_realization_settings&)
   * and takes a \p filename as paramter. The forth parameter is a pointer
   * to a string which can contain an error message in case the function
   * call fails. This can only be the case when the file cannot be
   * opened for writing.
   *
   * @section Example
   * The following example reads a realization from an existing file,
   * changes its version and header and writes
   * it back to the the same file.
   * @code
   * circuit circ;
   * std::string filename = "circuit.real";
   * std::string error;
   * 
   * if ( !read_realization( circ, filename, &error ) )
   * {
   *   std::cerr << error << std::endl;
   *   exit( 1 );
   * }
   *
   * write_realization_settings settings;
   * settings.version = "2.0";
   * settings.header = "Reversible Benchmarks\n(c) University";
   *
   * if ( !write_realization( circ, filename, settings, &error ) )
   * {
   *   std::cerr << error << std::endl;
   * }
   * @endcode
   *
   * @param circ     Circuit to write
   * @param filename Filename of the file to be created. The fill will be
   *                 overwritten in case it is already existing
   * @param settings Settings (see write_realization_settings)
   * @param error    If not-null, an error message is written
   *                 to this parameter in case the function fails
   *
   * @return true on success, false otherwise
   *
   * @author RevKit
   * @since  1.0
   */
  bool write_realization( const circuit& circ, const std::string& filename, const write_realization_settings& settings = write_realization_settings(), std::string* error = 0 );

}

#endif /* WRITE_REALIZATION_HPP */
