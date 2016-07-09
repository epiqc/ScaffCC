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
 * @file functor.hpp
 *
 * @brief Generic Functor Implementation based on <a href="// http://www.boost.org/doc/libs/1_43_0/doc/html/function.html" target="_blank">Boost.Function</a>.
 */

#ifndef FUNCTOR_HPP
#define FUNCTOR_HPP

#include <boost/function.hpp>

#include <core/properties.hpp>

namespace revkit
{

  /**
   * @brief Functor class for interfacing algorithms
   *
   * When interfacing an algorithm, we wanna encapsulate
   * the settings and the statistics. That is, a user can still
   * provide settings from outside, but another algorithm
   * can change settings of a functor as well. Therewith,
   * this class extends the boost::function object by adding
   * methods to access the corresponding settings and statistics
   * data from the respective algorithm.
   *
   * @author RevKit
   * @since  1.0
   */
  template<typename T>
  class functor : public boost::function<T>
  {
  public:
    /**
     * @brief Default constructor
     *
     * Calls the constructor of the base class.
     *
     * @author RevKit
     * @since  1.0
     */
    functor()
      : boost::function<T>() {}

    /** 
     * @brief Copy constructor
     *
     * This copy constructor allows for example, the assignment
     * of other boost::function objects or even boost::bind
     * or boost::lambda expressions. Note, that the settings
     * and statistics are not set with this constructor, but
     * have to be assigned explicitly using init().
     * 
     * @param f Object to be assigned
     *
     * @author RevKit
     * @since  1.0
     */
    template<typename F>
    functor( F f ) : boost::function<T>( f ) {}

    /** 
     * @brief Initializes the settings and statistics fields.
     *
     * This method is usually called by the functor creation function
     * to assign the settings and statistics data, e.g. in swop_func:
     *
     * @code
     * typedef functor<bool(circuit&, const binary_truth_table&)> truth_table_synthesis_func;
     *
     * ...
     *
     * truth_table_synthesis_func swop_func( properties::ptr settings, properties::ptr statistics )
     * {
     *   truth_table_synthesis_func f = boost::bind( swop, _1, settings, statistics );
     *   f.init( settings, statistics );
     *   return f;
     * }
     * @endcode
     * In this example, the settings and statistics objects are given as parameters to the swop function
     * in the boost::bind expression, so that they are available when calling the algorithm. Further
     * they are passed to the functor explicitly via init() to make them available using the functor,
     * e.g. in other algorithms.
     * 
     * @param settings Settings properties
     * @param statistics Statistics properties
     *
     * @author RevKit
     * @since  1.0
     */
    void init( const properties::ptr& settings, const properties::ptr& statistics )
    {
      _settings = settings;
      _statistics = statistics;
    }
    
    /** 
     * @brief Returns a smart pointer to the settings
     *
     * This smart pointer can be empty, if init() was never called. 
     * 
     * @return A smart pointer to the settings
     */
    const revkit::properties::ptr& settings() const
    {
      return _settings;
    }

    /** 
     * @brief Returns a smart pointer to the statistics
     *
     * This smart pointer can be empty, if init() was never called. 
     * 
     * @return A smart pointer to the statistics
     */
    const revkit::properties::ptr& statistics() const
    {
      return _statistics;
    }

  private:
    /** @cond */
    properties::ptr _settings;
    properties::ptr _statistics;
    /** @endcond */
  };

}

#endif /* FUNCTOR_HPP */
