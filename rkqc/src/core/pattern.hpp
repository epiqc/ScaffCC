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
 * @file pattern.hpp
 *
 * @brief Data Structure for Simulation Pattern
 */

#include <map>
#include <string>
#include <vector>

namespace revkit
{

  /**
   * @brief Pattern file for sequential simulation
   *
   * This class is used together with read_pattern and create_simulation_pattern
   * to create simulation pattern for sequential_simulation.
   *
   * @code
   * circuit circ;
   * // create circuit somehow
   *
   * pattern p;
   * read_pattern( p, "pattern.sim" );
   *
   * std::vector<boost::dynamic_bitset<> > sim;
   * std::map<std::string, boost::dynamic_bitset<> > init;
   * create_simulation_pattern( p, circ, sim, init );
   *
   * properties::ptr settings( new properties() );
   * settings->set( "initial_state", init );
   *
   * std::vector<boost::dynamic_bitset<> > outputs;
   * sequential_simulation( outputs, circ, sim, settings );
   * @endcode
   *
   * @author RevKit
   * @since  1.2
   */
  class pattern
  {
  public:
    /**
     * @brief Standard constructor
     *
     * @author RevKit
     * @since  1.2
     */
    pattern();

    /**
     * @brief Deconstructor
     *
     * @author RevKit
     * @since  1.2
     */
    ~pattern();

    /**
     * @brief Map for initializers
     *
     * The key is the name of the state signal and
     * the value is the assigned initial value.
     *
     * @author RevKit
     * @since  1.2
     */
    typedef std::map<std::string, unsigned> initializer_map;

    /**
     * @brief Pattern Type
     *
     * A pattern is a sequence of values
     * which are assigned to the input signals
     * at every step of simulation.
     *
     * @author RevKit
     * @since  1.2
     */
    typedef std::vector<unsigned> pattern_t;

    /**
     * @brief Vector of pattern
     *
     * @author RevKit
     * @since  1.2
     */
    typedef std::vector<pattern_t> pattern_vec;

    /**
     * @brief Adds an initializer
     *
     * @param name Name of the state signal
     * @param value Initial value
     *
     * @author RevKit
     * @since  1.2
     */
    void add_initializer( const std::string& name, unsigned value );

    /**
     * @brief Adds an input signal
     *
     * @param name Name of the input signal
     *
     * @author RevKit
     * @since  1.2
     */
    void add_input( const std::string& name );

    /**
     * @brief Adds a pattern sequence
     *
     * The pattern sequence must have the size of all
     * specified inputs.
     *
     * @param pattern Vector of pattern sequence
     *
     * @author RevKit
     * @since  1.2
     */
    void add_pattern( const std::vector<unsigned>& pattern );

    /**
     * @brief Returns the initializers
     *
     * A map is returned with a the name of the
     * state signal as key and the initial value
     * as key.
     *
     * @return Map of initializers
     */
    const initializer_map& initializers() const;

    /**
     * @brief Returns the list of input signals
     *
     * @return List of input signals
     */
    const std::vector<std::string>& inputs() const;

    /**
     * @brief Returns the list of pattern sequences
     *
     * @return List of pattern sequences
     */
    const pattern_vec& patterns() const;

  private:
    class priv;
    priv* const d;
  };

}

