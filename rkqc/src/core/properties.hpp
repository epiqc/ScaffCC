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
 * @file properties.hpp
 *
 * @brief Property Map Implementation for Algorithms
 *
 * @author RevKit
 * @since  1.0
 */

#ifndef PROPERTIES_HPP
#define PROPERTIES_HPP

#include <iostream>
#include <map>
#include <string>

#include <boost/any.hpp>
#include <boost/shared_ptr.hpp>

namespace revkit
{

  /**
   * @brief Property Map for storing settings and statistical information
   *
   * In this data structure settings and statistical data can be stored.
   * The key to access data is always of type \p std::string and the value
   * can be of any type. To be type-safe, the getter corresponding get
   * functions have to be provided with a type.
   *
   * @author RevKit
   * @since  1.0
   */
  struct properties
  {
    /**
     * @brief Internal storage type used with the internal property map
     *
     * @author RevKit
     * @since  1.0
     */
    typedef std::map<std::string, boost::any>                     storage_type;

    /**
     * @brief Value type of the property map, i.e. \p std::string
     *
     * @author RevKit
     * @since  1.0
     */
    typedef storage_type::mapped_type value_type;

    /**
     * @brief Key type of the property map, i.e. \p boost::any
     *
     * There are pre-defined getter methods, which can be called with a
     * type identifier for explicit casting.
     *
     * @author RevKit
     * @since  1.0
     */
    typedef storage_type::key_type   key_type;

    /**
     * @brief Smart Pointer version of this class
     *
     * Inside the framework, always the Smart Pointer version is used.
     * To have an easy access, there are special functions provided
     * which take the smart pointer as parameter and check as well
     * if it can be dereferenced.
     *
     * @sa get
     * @sa set_error_message
     *
     * @author RevKit
     * @since  1.0
     */
    typedef boost::shared_ptr<properties>                         ptr;

    /**
     * @brief Standard constructor
     *
     * Creates the property map on base of the storage map
     *
     * @author RevKit
     * @since  1.0
     */
    properties();

    /**
     * @brief Direct access to the value type
     *
     * Since the \p value_type is of type \p boost::any, it is not recommended
     * to use this operator, but rather get and set.
     *
     * @param k Key to access the property map. Must exist.
     * @return The value associated with key \p k.
     *
     * @author RevKit
     * @since  1.0
     */
    const value_type& operator[]( const key_type& k ) const;

    /**
     * @brief Casted access to an existing element
     *
     * With \p T you can specify the type of the element. Note, that
     * it has to be the original used type, e.g. there is a difference
     * even between \p int and \p unsigned.
     *
     * The type is determined automatically using the set method.
     *
     * @param k Key to access the property map. Must exist.
     * @return The value associated with key \p k casted to its original type \p T.
     *
     * @author RevKit
     * @since  1.0
     */
    template<typename T>
    T get( const key_type& k ) const
    {
      return boost::any_cast<T>( map.find( k )->second );
    }

    /**
     * @brief Casted access to an existing element with fall-back option
     *
     * The same as get(const key_type& k), but if \p k does not exist,
     * a default value is returned, which has to be of type \p T.
     *
     * @param k Key to access the property map. May not exist.
     * @param default_value If \p k does not exist, this value is returned.
     * @return The value associated with key \p k casted to its original type \p T. If the key \p k does not exist,
     *         \p default_value is returned.
     *
     * @author RevKit
     * @since  1.0
     */
    template<typename T>
    T get( const key_type& k, const T& default_value ) const
    {
      if ( map.find( k ) == map.end() )
      {
        return default_value;
      }
      else
      {
        return boost::any_cast<T>( map.find( k )->second );
      }
    }

    /**
     * @brief Adds or modifies a value in the property map
     *
     * This methods sets the value located at key \p k to \p value.
     * If the key does not exist, it will be created.
     * Be careful which type was used, especially with typed constants:
     * @code
     * properties p;
     * p.set( "a unsigned number", 5u );
     * p.get<unsigned>( "a unsigned number" ); // OK!
     * p.get<int>( "a unsigned number" );      // FAIL!
     *
     * p.set( "a signed number", 5 );
     * p.get<unsigned>( "a signed number" );   // FAIL!
     * p.get<int>( "a signed number" );        // OK!
     * @endcode
     *
     * @param k Key of the property
     * @param value The new value of \p k. If \p k already existed, the type of \p value must not change.
     *
     * @author RevKit
     * @since  1.0
     */
    void set( const key_type& k, const value_type& value );

    /**
     * @brief Start iterator for the properties
     *
     * @return Iterator
     *
     * @author RevKit
     * @since  1.1
     */
    storage_type::const_iterator begin() const;

    /**
     * @brief End iterator for the properties
     *
     * @return Iterator
     *
     * @author RevKit
     * @since  1.1
     */
    storage_type::const_iterator end() const;

    /**
     * @brief Number of properties
     *
     * @return The number of properties
     *
     * @author RevKit
     * @since  1.0
     */
    unsigned size() const;

    /**
     * @brief Clears all properties
     *
     * @author RevKit
     * @since  1.1
     */
    void clear();

  private:
    storage_type      map;
  };

  /**
   * @brief A helper method to access the get method on a properties smart pointer
   *
   * This method has basically two fall backs. If settings does not point to anything,
   * it returns \p default_value, and otherwise it calls the get method on the
   * pointee of the smart pointer with the \p default_value again, so in case the key \p k
   * does not exists, the \p default_value is returned as well.
   *
   * @param settings A smart pointer to a properties instance or an empty smart pointer
   * @param k Key of the property to be accessed
   * @param default_value A default_value as fall back option in case the smart pointer
   *                      is empty or the key does not exist.
   *
   * @return The value addressed by \p k or the \p default_value.
   *
   * @author RevKit
   * @since  1.0
   */
  template<typename T>
  T get( const properties::ptr& settings, const properties::key_type& k, const T& default_value )
  {
    return settings ? settings->get<T>( k, default_value ) : default_value;
  }

  /**
   * @brief Sets an error message to a statistics smart pointer
   *
   * This function checks first if the smart pointer references something,
   * and if that is the case, the value \p error, is written to the key
   * \b error.
   *
   * @param statistics A smart pointer to a properties instance or an empty smart pointer
   * @param error An error message, which should be written to the key \b error
   *              if the smart pointer \p statistics can be de-referenced.
   *
   * @author RevKit
   * @since  1.0
   */
  void set_error_message( properties::ptr statistics, const std::string& error );

}

#endif /* PROPERTIES_HPP */
