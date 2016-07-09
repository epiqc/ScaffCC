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
 * @file gate.hpp
 *
 * @brief Gate class
 */

#ifndef GATE_HPP
#define GATE_HPP

//#define CHANGE

#include <iostream>
#include <set>
#include <vector>


#include <boost/unordered_set.hpp>
#include <boost/any.hpp>
#include <boost/unordered_set.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/utility.hpp>

namespace revkit
{

  struct transform_line;
  struct filter_line;
  class filtered_gate;

  /**
   * @brief Represents a gate in a circuit
   *
   * @author RevKit
   * @since  1.0
   */
  class gate  
  {
  public:
    /**
     * @brief Vector type of gates
     *
     * @author RevKit
     * @since  1.0
     */
    typedef std::vector<gate>  vector;

    /**
     * @brief Type for accessing the line (line index)
     *
     * @author RevKit
     * @since  1.0
     */
    typedef unsigned                line;

    /**
     * @brief Container for storing lines
     *
     * @author RevKit
     * @since  1.0
     */
    typedef std::set<line>          line_container;

	#ifdef CHANGE
    typedef std::vector<line>          lines_container;
	#endif


    /**
     * @brief Mutable Iterator for iterating through control or target lines
     *
     * @author RevKit
     * @since  1.0
     */
    typedef boost::transform_iterator<transform_line, boost::filter_iterator<filter_line, line_container::iterator> > iterator;
	#ifdef CHANGE
    typedef std::vector<line>::iterator iterator;
	#endif

    /**
     * @brief Constant Iterator for iterating through control or target lines
     *
     * @author RevKit
     * @since  1.0
     */
    typedef boost::transform_iterator<transform_line, boost::filter_iterator<filter_line, line_container::const_iterator> > const_iterator;
	#ifdef CHANGE
    typedef std::vector<line>::const_iterator const_iterator;
	#endif

  public:
    /**
     * @brief Default constructor
     *
     * Initializes private data
     *
     * @author RevKit
     * @since  1.0
     */
    gate();

    /**
     * @brief Copy Constructor
     *
     * Initializes private data and copies gate
     *
     * @param other Gate to be assigned
     *
     * @author RevKit
     * @since  1.0
     */
    gate( const gate& other );

    /**
     * @brief Default deconstructor
     *
     * Clears private data
     *
     * @author RevKit
     * @since  1.0
     */
    virtual ~gate();

    /**
     * @brief Assignment operator
     *
     * @param other Gate to be assigned
     *
     * @return Pointer to instance
     *
     * @author RevKit
     * @since  1.0
     */
    gate& operator=( const gate& other );
    
    /**
     * @brief Start iterator for accessing control lines. 
     *
     * Returns The start iterator of the line_container for accessing control lines. 
     *
     * @author RevKit
     * @since 1.0
     */
    virtual const_iterator begin_controls() const;

    /**
     * @brief End iterator for accessing control lines (const).
     *
     * Returns The end iterator of the line_container for accessing control lines (const).
     *
     * @author RevKit
     * @since 1.0
     */
    virtual const_iterator end_controls() const;

    /**
     * @brief Start iterator for accessing control lines (non-const).
     *
     * Returns The start iterator of the line_container for accessing lines (non-const). 
     *
     * @author RevKit
     * @since 1.0
     */
    virtual iterator begin_controls();

    /**
     * @brief End iterator for accessing control lines (non-const).
     *
     * Returns the end iterator of the line_container for accessing control lines (non-const). 
     *
     * @author RevKit
     * @since 1.0
     */
    virtual iterator end_controls();

    /**
     * @brief Start iterator for accessing target lines (const).
     *
     * Returns The start iterator of the line_container for accessing target lines (const). 
     *
     * @author RevKit
     * @since 1.0
     */
    virtual const_iterator begin_targets() const;

    /**
     * @brief End iterator for accessing target lines (const).
     *
     * Returns The end iterator of the line_container for accessing target lines (const). 
     *
     * @author RevKit
     * @since 1.0
     */
    virtual const_iterator end_targets() const;


     /**
     * @brief Start iterator for accessing target lines (const).
     *
     * Returns The start iterator of the line_container for accessing target lines (const). 
     *
     * @author RevKit
     * @since 1.0
     */
    virtual iterator begin_targets();

    /**
     * @brief End iterator for accessing target lines (non-const).
     *
     * Returns The start iterator of the line_container for accessing target lines (non-const). 
     *
     * @author RevKit
     * @since 1.0
     */
    virtual iterator end_targets();

    /**
     * @brief Returns the number of control and target lines as sum
     *
     * This method returns the number of control and target
     * lines as sum and can be used for e.g. calculating costs.
     *
     * @author RevKit
     * @since  1.0
     *
     * @return Number of control and target lines.
     */
    virtual unsigned size() const;

    /** 
     * @brief Adds a control line to the gate
     *
     * @param c control line to add
     *
     * @author RevKit
     * @since 1.0 
     */
    virtual void add_control( line c );

    /** 
     * @brief Remove control line to the gate
     *
     * @param c control line to remove
     *
     * @author RevKit
     * @since 1.0 
     */
    virtual void remove_control( line c );

    /**
     * @brief Adds a target to the desired line
     *
     * @param l target line 
     *
     * @author RevKit
     * @since 1.0
     */
    virtual void add_target( line l );
 
    /**
     * @brief Removes a target from the desired line
     *
     * @param l target line 
     *
     * @author RevKit
     * @since 1.0
     */
    virtual void remove_target( line l );
    
    /**
     * @brief Sets the type of the target line(s)
     *
     * @param t target type
     *
     * @author RevKit
     * @since  1.0
     */
    virtual void set_type( const boost::any& t );

    /**
     * @brief Returns the type of the target line(s)
     *
     * @return target type
     *
     * @author RevKit
     * @since  1.0
     */
    virtual const boost::any& type() const;

    friend class filtered_gate;
	
	std::vector<unsigned> controls_ordered( void ) const;
	std::vector<unsigned> targets_ordered( void ) const;

  private:
    class priv;
    priv* const d;
  };

  /**
   * @brief Wrapper for a gate to filter some lines
   *
   * This class wraps a underline \p base gate to just
   * access some lines which are specified in filter.
   *
   * You will never have to create a filtered_gate object
   * on your own, but you will get this as reference object
   * to your iterators in a subcircuit.
   *
   * @author RevKit
   * @since  1.0
   */
  class filtered_gate : public gate
  {
  public:
    /**
     * @brief Standard constructor
     *
     * Creates a filtered_gate from a base gate and
     * a list of indices which should be included in this
     * gate.
     * 
     * @param base   The underlying referenced gate
     * @param filter A vector with line indices which are included in this gate
     *
     * @author RevKit
     * @since  1.0
     */
    filtered_gate( gate& base, std::vector<unsigned>& filter );

    /**
     * @brief Copy constructor
     *
     * @param other Object to be copied from
     *
     * @author RevKit
     * @since  1.0
     */
    explicit filtered_gate( const filtered_gate& other );

    /**
     * @brief Deconstructor
     */
    virtual ~filtered_gate();

    /**
     * @brief Assignment operator
     *
     * @param other Gate to be assigned
     *
     * @return Pointer to instance
     *
     * @author RevKit
     * @since  1.0
     */
    filtered_gate& operator=( const filtered_gate& other );

    const_iterator begin_controls() const;
    const_iterator end_controls() const;
    iterator begin_controls();
    iterator end_controls();

    const_iterator begin_targets() const;
    const_iterator end_targets() const;
    iterator begin_targets();
    iterator end_targets();

    unsigned size() const;
    void add_control( line c );
    void remove_control( line c );
    void add_target( line l );
    void remove_target( line l );

    void set_type( const boost::any& t );
    const boost::any& type() const;

  private:
    class priv;
    priv* const d;
  };

  /** @cond */
  struct transform_line
  {
    typedef gate::line result_type;
    
    transform_line() : filter( 0 ) {}

    explicit transform_line( const std::vector<unsigned>& filter ) : filter( &filter ) {}

    gate::line operator()( gate::line l ) const
    {
      return filter ? std::find( filter->begin(), filter->end(), l ) - filter->begin() : l;
      return l;
    }

  private:
    const std::vector<unsigned>* filter;
  };
  /** @endcond */

  /** @cond */
  struct filter_line
  {
    filter_line() : filter( 0 ) {}

    explicit filter_line( const std::vector<unsigned>& filter ) : filter( &filter ) {}

    bool operator()( const gate::line& l ) const
    {
      return !filter || std::find( filter->begin(), filter->end(), l ) != filter->end();
    }

  private:
    const std::vector<unsigned>* filter;
  };
  /** @endcond */

}

#endif /* GATE_HPP */
