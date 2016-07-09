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
 * @file timer.hpp
 *
 * @brief A generic way for measuring time
 */

#ifndef TIMER_HPP
#define TIMER_HPP

#include <cassert>
#include <iostream>

#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <sys/times.h>

#include <core/properties.hpp>

namespace revkit {

  /**
   * @brief Functor for the timer class which prints the run-time to an output stream
   *
   * This functor prints the run-time to a given output stream which can be
   * specified in the constructor.
   *
   * @author RevKit
   * @since  1.0
   */
  struct print_timer {
    /**
     * Result value of the print_timer is void,
     * since it does not return anything in the operator call.
     *
     * @sa timer::operator()()
     *
     * @author RevKit
     * @since  1.0
     */
    typedef void result_type;

    /** 
     * @brief Default constructor
     *
     * Available for delayed starting of the timer.
     *
     * @author RevKit
     * @since  1.0
     */
    print_timer() : os( std::cout ) {}

    /**
     * @brief Default constructor
     *
     * @param _os Stream where to write the run-time after measuring
     *
     * @author RevKit
     * @since  1.0
     */
    explicit print_timer( std::ostream& _os = std::cout ) : os( _os ) {}

    /**
     * @brief Prints the measured run-time
     *
     * @param runtime The run-time
     *
     * @author RevKit
     * @since  1.0
     */
    void operator()( double runtime ) const {
      std::cout << "Runtime: " << runtime << " secs" << std::endl;
    }

  private:
    std::ostream& os;
  };

  /**
   * @brief Functor for the timer class which assigns the run-time to a given variable
   *
   * Make sure that the variable for the run-time is declared outside of the scope
   * of time measuring.
   *
   * @section sec_example_reference_timer Example
   *
   * @code
   * double runtime;
   * {
   *   revkit::reference_timer rt( &runtime );
   *   revkit::timer<revkit::reference_timer> t( rt );
   *   // code for which time should be measured
   * }
   * @endcode
   *
   * @author RevKit
   * @since  1.0
   */
  struct reference_timer {
    /**
     * Result value of the reference_timer is double,
     * since it returns the value of the reference value, the run-time
     * in the operator call. This is only useful, when using the intermediate measurement
     * in timer.
     *
     * @sa timer::operator()()
     *
     * @author RevKit
     * @since  1.0
     */
    typedef double result_type;

    /** 
     * @brief Default constructor
     *
     * Available for delayed starting of the timer.
     *
     * @author RevKit
     * @since  1.0
     */
    reference_timer() {}

    /**
     * @brief Default constructor
     *
     * @param _runtime A pointer referencing to the variable where the run-time should be saved.
     *
     * @author RevKit
     * @since  1.0
     */
    explicit reference_timer( double* _runtime ) : runtime( _runtime ) {}

    /**
     * @brief Saves the run-time to the variable
     *
     * @param r The run-time
     *
     * @author RevKit
     * @since  1.0
     */
    double operator()( double r ) const {
      return ( *runtime = r );
    }

  private:
    double* runtime;
  };

  /**
   * @brief Functor for the timer class which assigns the run-time to a property map
   *
   * This functor writes the \em runtime field of a property map
   * after the time was measured and is thus similar to the
   * reference_timer.
   *
   * @author RevKit
   * @since  1.0
   */
  struct properties_timer
  {

    /**
     * Result value of the reference_timer is double,
     * since it returns the value of the run-time
     * in the operator call. This is only useful, when using the intermediate measurement
     * in timer.
     *
     * @sa timer::operator()()
     *
     * @author RevKit
     * @since  1.0
     */
    typedef double result_type;

    /** 
     * @brief Default constructor
     *
     * Available for delayed starting of the timer.
     *
     * @author RevKit
     * @since  1.0
     */
    properties_timer() {}

    /**
     * @brief Default constructor
     *
     * @param _statistics A smart pointer to a statistics properties object. Can be empty.
     *
     * @author RevKit
     * @since  1.0
     */
    properties_timer( properties::ptr _statistics ) : statistics( _statistics ) {}

    /**
     * @brief Saves the run-time to the \b runtime field of the statistics variable
     *
     * @param r The run-time
     *
     * @author RevKit
     * @since  1.0
     */
    double operator()( double r ) const
    {
      if ( statistics )
      {
        statistics->set( "runtime", r );
      }
      return r;
    }

  private:
    properties::ptr statistics;
  };

  /**
   * @brief Measure Method for timer
   *
   * This class is only holding one enumeration
   * used for the timer::set_measure_method method.
   */
  struct measure_method
  {
    /**
     * @brief Flags for different times to measure
     *
     * Flags which can be used (also in combination) by the
     * timer::set_measure_method method.
     *
     * @author RevKit
     * @since  1.1
     */
    enum { none = 0, user_time = 1, system_time = 2 };
  };

  /**
   * @brief A generic timer class
   *
   * This class measures the time between its constructor and
   * its deconstructor is called. In the deconstructor a
   * functor is called which can be assigned in the constructor.
   *
   * Because of this design the code for which the run-time has
   * to be checked has to be enclosed as a block and the timer
   * needs to be initialized as local variable in the beginning
   * of the block, so that the deconstructor get called automatically
   * at the end of the block.
   *
   * The Outputter has to implement <b>operator()(double runtime) const</b>.
   *
   * Two functors are already implemented in the library:
   * - print_timer: Prints the run-time to a given output stream
   * - reference_timer: Assigns the run-time to a variable
   *
   * @section sec_example_timer Example
   *
   * This example demonstrates how to create a timer functor which
   * outputs the run-time to STDOUT and how to use the timer class
   * with that functor.
   *
   * It is important to specify a result_type which is the result of the
   * () operator.
   *
   * @sa timer::operator()()
   *
   * @code
   * #include <core/utils/timer.hpp>
   *
   * struct output_timer {
   *   typedef void result_type;
   *
   *   void operator()( double runtime ) const {
   *     std::cout << runtime << std::endl;
   *   }
   * };
   *
   * ...
   *
   * // some other code for which no time should be measured
   * {
   *   output_timer ot;
   *   revkit::timer<output_timer> t( ot );
   *   // code for which time should be measured
   * }
   * // some other code for which no time should be measured
   *
   * 
   * @endcode
   *
   * @author RevKit
   * @since  1.0
   */
  template<typename Outputter>
  class timer
  {
  public:
    /**
     * @brief Constructor which does not start measuring the time
     *
     * When delayed starting should be done (using start(const Outputter&)) this
     * constructor has to be used.
     *
     * @author RevKit
     * @since  1.0
     */
    timer()
      : started( false ),
        _measure_method( measure_method::user_time )
    {
    }

    /**
     * @brief Constructor which starts measuring the time
     *
     * The \p outputter is copied and called in the
     * deconstructor with the measured time.
     *
     * @param outputter Functor which does something with the measured run-time
     *
     * @author RevKit
     * @since  1.0
     */
    explicit timer( const Outputter& outputter )
      : p( outputter ),
        started( true ),
        _measure_method( measure_method::user_time )
    {
      times( &_start );
    }

    /**
     * @brief Delayed start
     *
     * There might be reasons when the starting of the measurement
     * should be delayed and not started with the constructor.
     * For this cases this method can be used.
     *
     * @param outputter Functor which does something with the measured run-time
     *
     * @author RevKit
     * @since  1.0
     */
    void start( const Outputter& outputter )
    {
      p = outputter;
      started = true;
      times( &_start );
    }

    /**
     * @brief Specify the measure method
     *
     * Use the flags of the timer class to specify the measure method,
     * e.g. measure_method::user_time or measure_method::system_time. The flags can also
     * be merged to capture both and return the sum, i.e. measure_method::user_time | measure_method::system_time.
     *
     * @author RevKit
     * @since  1.1
     */
    void set_measure_method( unsigned method )
    {
      _measure_method = method;
    }

    /**
     * @brief Intermediate Call of the timer functor
     *
     * This operator also returns the return type of the functor.
     * This is useful, when using the reference_timer which returns
     * the value of the reference in the functor call.
     *
     * @section sec_example_timer_intermediate Example
     * In this function the timer is used explicitly without a scope
     * and the start method as well as the intermediate operator.
     *
     * @code
     * double runtime;
     * reference_timer rt( &runtime );
     * timer<reference_timer> t;
     * // before starting the measurement
     * t.start( rt );
     * // calculate a bit
     * double intermediate_time = t();
     * // calculate a bit more
     * intermediate_time = t();
     * @endcode
     *
     * @return The result value of the functor operator (if available)
     *
     * @author RevKit
     * @since  1.0
     */
    typename Outputter::result_type operator()() const
    {
      assert( started );

      struct tms end;
      times( &end );

      clock_t end_t = 0, start_t = 0;

      if ( _measure_method & measure_method::user_time )
      {
        end_t += end.tms_utime + end.tms_cutime;
        start_t += _start.tms_utime + _start.tms_cutime;
      }
      else if ( _measure_method & measure_method::system_time )
      {
        end_t += end.tms_stime + end.tms_cstime;
        start_t += _start.tms_stime + _start.tms_cstime;
      }

      double runtime = ( double( end_t - start_t ) ) / (double)sysconf( _SC_CLK_TCK );
      return p( runtime );
    }

    /**
     * @brief Deconstructor which stops measuring the time
     *
     * In this function time is measured again and the functor
     * is called with the runtime.
     *
     * @author RevKit
     * @since  1.0
     */
    virtual ~timer() {
      if ( started )
      {
        operator()();
      }
    }

  private:
    struct tms _start;
    Outputter p; // NOTE: has to be copy
    bool started;
    unsigned _measure_method;
  };

}

#endif
