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
 * @file bus_collection.hpp
 *
 * @brief Bus Collection
 */
#ifndef BUS_COLLECTION_HPP
#define BUS_COLLECTION_HPP

#include <map>
#include <string>
#include <vector>

#include <boost/optional.hpp>

namespace revkit
{

  /**
   * @brief Collection for buses
   *
   * This class represents a collection of buses.
   * Using respective methods it is possible to add
   * new buses or find buses by name or line index.
   *
   * Buses are usually accessed via the methods circuit::inputbuses()
   * and circuit::outputbuses()
   *
   * @author RevKit
   * @since  1.1
   */
  class bus_collection
  {
  public:
    /**
     * @brief
     *
     * @author RevKit
     * @since  1.1
     */
    typedef std::map<std::string, std::vector<unsigned> > map;

    /**
     * @brief Standard constructor
     *
     * @author RevKit
     * @since  1.1
     */
    bus_collection();

    /**
     * @brief Deconstructor
     *
     * @author RevKit
     * @since  1.1
     */
    ~bus_collection();

    /**
     * @brief Adds a new bus to the collection
     *
     * @param name Name of the bus
     * @param line_indices Corresponding lines of the bus
     * @param initial_value This value can optional be set
     *                      to assign an initial value to
     *                      this bus.
     *
     * @author RevKit
     * @since  1.1
     */
    void add( const std::string& name, const std::vector<unsigned>& line_indices, const boost::optional<unsigned>& initial_value = boost::optional<unsigned>() );

    /**
     * @brief Gets the corresponding lines of a bus by the name
     *
     * If there is no such bus with the name \p name in the collection,
     * an assertion is thrown. This method is meant to be used in conjunction
     * with find_bus.
     *
     * @param name Name of the bus
     *
     * @return Corresponding lines of the bus
     *
     * @author RevKit
     * @since  1.1
     */
    const std::vector<unsigned>& get( const std::string& name ) const;

    /**
     * @brief Returns all buses of the collection
     *
     * This method returns all the buses of the collection.
     *
     * @return All buses of the collection
     *
     * @author RevKit
     * @since  1.1
     */
    const map& buses() const;

    /**
     * @brief This method finds the bus for a line
     *
     * If the line belongs to a bus, the name of the bus
     * is returned, otherwise an empty string is returned.
     *
     * If the name is not important, but just the check whether the
     * line is contained in some bus, use this method rather than bus_collection::find_bus
     *
     * @param line_index Index of the line
     *
     * @return Name of the bus, or empty string in case the line does not belong to any bus
     *
     * @author RevKit
     * @since  1.1
     */
    std::string find_bus( unsigned line_index ) const;

    /**
     * @brief This method determines whether there exists a bus for a given line
     *
     * If the line at \p line_index is contained in a bus, this
     * method returns \b true, otherwise \b false.
     *
     * If the name is not important, but just the check whether the
     * line is contained in some bus, use this method rather than bus_collection::find_bus
     *
     * @param line_index Index of the line
     *
     * @return \b true, if line at \p line_index is contained in a bus
     */
    bool has_bus( unsigned line_index ) const;

    /**
     * @brief Returns the signal index relative to the bus
     *
     * If e.g. a bus \b A is defined by the line indices 3,4,6
     * then the signal index of 4 is 1, since it is the 2nd signal
     * in the bus (considering counting from 0).
     *
     * This method requires, that \p line_index belongs to a bus,
     * otherwise an assertion is thrown.
     *
     * @param line_index Index of the line
     *
     * @return Index of the signal relative to the bus
     *
     * @author RevKit
     * @since  1.1
     */
    unsigned signal_index( unsigned line_index ) const;

    /**
     * @brief Sets the initial value of a bus
     *
     * This method is used primarily for state signals, which
     * can be assigned an initial value. This method is called
     * with the name of the bus. If no such bus exists, this method
     * call has no effect.
     *
     * @param name Name of the bus
     * @param initial_value Initial value
     *
     * @author RevKit
     * @since  1.1
     */
    void set_initial_value( const std::string& name, unsigned initial_value );

    /**
     * @brief Retrieves the initial value of a bus
     *
     * Given a name of the bus, this method tries to retrieve an initial
     * value. If no bus with this name exists, or if a bus exist but
     * does not have an initial value, an empty optional is returned.
     * Thus, this method should be used as demonstrated in the following example:
     * @code
     * boost::optional<unsigned> iv = bus.initial_value( "bus_name" );
     * if ( iv )
     * {
     *   // bus has the initial value *iv
     *   std::cout << "Initial value: " << iv << std::endl;
     * }
     * @endcode
     * If there exists a default value for the initial value, the following
     * shorter snippet can be used:
     * @code
     * unsigned iv = bus.initial_value( "bus_name" ).get_value_or( 0u );
     * @endcode
     *
     * @param name Name of the bus
     *
     * @return Initial value
     *
     * @author RevKit
     * @since  1.1
     */
    boost::optional<unsigned> initial_value( const std::string& name ) const;

  private:
    class priv;
    priv* const d;
  };

}

#endif /* BUS_HPP */
