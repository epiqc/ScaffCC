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
 * @file circuit.hpp
 *
 * @brief Circuit class
 */

#ifndef CIRCUIT_HPP
#define CIRCUIT_HPP

#include <map>

/**
 * For a more convenient use in foreach loops.
 * You can use the name foreach instead of BOOST_FOREACH.
 *
 * @author RevKit
 * @since  1.0
 */
#define foreach BOOST_FOREACH

#include <boost/foreach.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/iterator/indirect_iterator.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/signal.hpp>
#include <boost/variant.hpp>

#include <core/gate.hpp>
#include <core/meta/bus_collection.hpp>




namespace revkit
{


  /**
   * @brief Type for determine whether line is constant or not
   *
   * The following table summarizes the use of constant values
   * in circuit representations.
   *
   * <table border="0">
   *   <tr>
   *     <td class="indexkey">Description</td>
   *     <td class="indexkey">Char representation</td>
   *     <td class="indexkey">Typed value</td>
   *   </tr>
   *   <tr>
   *     <td class="indexvalue">No constant input line</td>
   *     <td align="center" class="indexvalue">'-'</td>
   *     <td class="indexvalue">@code constant() @endcode</td>
   *   </tr>
   *   <tr>
   *     <td class="indexvalue">Constant input line with value 0</td>
   *     <td align="center" class="indexvalue">'0'</td>
   *     <td class="indexvalue">@code constant( 0 ) @endcode</td>
   *   </tr>
   *   <tr>
   *     <td class="indexvalue">Constant input line with value 1</td>
   *     <td align="center" class="indexvalue">'1'</td>
   *     <td class="indexvalue">@code constant( 1 ) @endcode</td>
   *   </tr>
   * </table>
   *
   * @section Example
   * This example demonstrates how to access the constant values.
   * @code
   * constant c = // some constant
   *
   * if ( c ) // checks whether c is set or not
   * {
   *   if ( *c ) // c is checked, checks the value of c
   *   {
   *     std::cout << "constant value 1" << std::endl;
   *   }
   *   else
   *   {
   *     std::cout << "constant value 0" << std::endl;
   *   }
   * }
   * else
   * {
   *   std::cout << "no constant value" << std::endl;
   * }
   * @endcode
   *
   * @author RevKit
   * @since  1.0
   */
  typedef boost::optional<bool> constant;

  /**
   * @brief Represents a circuit
   *
   * A circuit is represented by a list of gates (type gate::vector)
   * and meta information like name, inputs, outputs, constants,
   * and garbage.
   *
   * Via STL like iterators the gates can be accessed and also used
   * in STL and STL like algorithms based on the iterator concept.
   *
   * @section example_circuit_class_1 Example: Creating a circuit with 5 lines
   * @code
   * #include <core/circuit.hpp>
   *
   * ...
   *
   * circuit circ( 5 );
   * @endcode
   *
   * @section example_circuit_class_2 Example: Iterate through all gates in a circuit \p circ with iterators
   * @code
   * for ( circuit::const_iterator itGate = circ.begin(); itGate != circ.end(); ++itGate )
   * {
   *   gate& g = *itGate;
   * }
   * @endcode
   *
   * @section example_circuit_class_3 Example: Iterator through all gates in a circuit with foreach
   * @code
   * #include <boost/foreach.hpp>
   * #define foreach BOOST_FOREACH
   *
   * ...
   *
   * foreach ( gate& g, circ )
   * {
   *   // g can be modified
   * } 
   *
   * foreach ( const gate& g, circ )
   * {
   *   // g cannot be modified
   * } 
   * @endcode
   *
   * @author RevKit
   * @since  1.0
   */
  class standard_circuit
  {
  public:
    /**
     * @brief Default Constructor
     *
     * Creates an empty circuit with zero lines.
     *
     * @author RevKit
     * @since  1.0
     */
    standard_circuit() : lines( 0 ) {}

    /**
     * @brief Default Constructor
     *
     * Creates an empty circuit with \p lines lines.
     *
     * @param lines Number of lines
     *
     * @author RevKit
     * @since  1.0
     */
    standard_circuit( unsigned lines ) : lines( lines )
    {
      inputs.resize( lines, "i" );
      outputs.resize( lines, "o" );
      constants.resize( lines, constant() );
      garbage.resize( lines, false );
    }

    /** @cond */
    std::vector<boost::shared_ptr<gate> > gates;
    unsigned lines;

    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
    std::vector<constant> constants;
    std::vector<bool> garbage;

    std::string name;
    bus_collection inputbuses;
    bus_collection outputbuses;
    bus_collection statesignals;
    std::map<const gate*, std::map<std::string, std::string> > annotations;
    /** @endcond */
  };

  class subcircuit;

  /**
   * @brief Generic circuit
   *
   * This circuit can be both standard_circuit or a subcircuit.
   * In the default case it is considered as standard_circuit with
   * no lines.
   *
   * In the following examples are given how to construct different
   * types of circuits.
   *
   * <b>Empty circuit with no lines</b>
   * @code circuit circ; @endcode
   *
   * <b>Empty circuit with 3 lines</b>
   * @code circuit circ( 3 ); @endcode
   *
   * <b>Sub-circuit from \p circ including gates [2,4)</b>
   * @code subcircuit subcirc( circ, 2, 4 ); @endcode
   *
   * @author RevKit
   * @since  1.0
   */
  typedef boost::variant<standard_circuit, subcircuit> circuit_variant;

  /**
   * @brief Represents a sub-circuit
   *
   * A sub-circuit is a <i>window</i> which can be set
   * on top of a standard_circuit. All methods are
   * specialized for the sub-circuit. But the gates are
   * references to the underlying circuit.
   */
  class subcircuit
  {
  public:
    /**
     * @brief Default constructor
     *
     * This constructor creates a sub-circuit from a
     * base which is a standard_circuit and a range of gates
     * [\p from, \p to). Thus, the gate with index \p to is
     * not included.
     *
     * @param base Underlying circuit
     * @param from First gate to be included (starting from 0)
     * @param to First gate to be not included anymore
     *
     * @author RevKit
     * @since  1.0
     */
    subcircuit( standard_circuit& base, unsigned from, unsigned to ) : base( base ), from( from ), to( to ) {}

    /**
     * @brief Default constructor
     *
     * Same as other constructor but takes a generic circuit,
     * which will get casted to a standard_circuit.
     *
     * @param base Underlying circuit (has to be standard_circuit in the variant)
     * @param from First gate to be included (starting from 0)
     * @param to First gate to be not included anymore
     *
     * @author RevKit
     * @since  1.0
     */
    subcircuit( circuit_variant& base, unsigned from, unsigned to ) : base( boost::get<standard_circuit>( base ) ), from( from ), to( to ) {}

    /**
     * @brief Default constructor
     *
     * Same as other constructor but takes a generic circuit,
     * which will get casted to a standard_circuit.
     *
     * @param base Underlying circuit (has to be standard_circuit in the variant)
     * @param from First gate to be included (starting from 0)
     * @param to First gate to be not included anymore
     *
     * @author RevKit
     * @since  1.0
     */
    subcircuit( const circuit_variant& base, unsigned from, unsigned to ) : base( boost::get<standard_circuit>( const_cast<circuit_variant&>( base ) ) ), from( from ), to( to ) {}

    /**
     * @brief Constructor with line filter
     *
     * This constructor creates a sub-circuit from a
     * base which is a standard_circuit and a range of gates
     * [\p from, \p to). Thus, the gate with index \p to is
     * not included.
     *
     * Further a line filter is specified. The vector contains all
     * line indices (starting from 0) which should be accessible.
     *
     * @param base Underlying circuit
     * @param from First gate to be included (starting from 0)
     * @param to First gate to be not included anymore
     * @param filter Line filter
     *
     * @author RevKit
     * @since  1.0
     */
    subcircuit( standard_circuit& base, unsigned from, unsigned to, std::vector<unsigned> filter ) : base( base ), from( from ), to( to ), filter( filter )
    {
      std::sort( this->filter.begin(), this->filter.end() );
      this->filter.resize( std::unique( this->filter.begin(), this->filter.end() ) - this->filter.begin() );
    }

    /**
     * @brief Constructor with line filter
     *
     * Same as other constructor but takes a generic circuit,
     * which will get casted to a standard_circuit.
     *
     * @param base Underlying circuit (has to be a standard_circuit in the variant)
     * @param from First gate to be included (starting from 0)
     * @param to First gate to be not included anymore
     * @param filter Line filter
     *
     * @author RevKit
     * @since  1.0
     */
    subcircuit( const circuit_variant& base, unsigned from, unsigned to, std::vector<unsigned> filter ) : base( boost::get<standard_circuit>( const_cast<circuit_variant&>( base ) ) ), from( from ), to( to ), filter( filter )
    {
      std::sort( this->filter.begin(), this->filter.end() );
      this->filter.resize( std::unique( this->filter.begin(), this->filter.end() ) - this->filter.begin() );
    }

    /**
     * @brief Deconstructor
     *
     * Deletes all filtered_gate objects in the filter cache
     *
     * @author RevKit
     * @since  1.0
     */
    ~subcircuit()
    {
      std::map<gate*, filtered_gate*>::const_iterator it;
      for ( it = filter_cache.begin(); it != filter_cache.end(); ++it )
      {
        delete it->second;
      }
    }

    /** @cond */
    standard_circuit& base;
    unsigned from;
    unsigned to;
    std::vector<unsigned> filter;
    
    // for the transform_iterator implementation
    std::map<gate*, filtered_gate*> filter_cache;
    /** @endcond */
  };

  /** @cond */
  struct filter_circuit
  {
    typedef gate& result_type;

    filter_circuit() : circ( 0 ) {}
    explicit filter_circuit( subcircuit& circ ) : circ( &circ ) {}

    gate& operator()( gate& g ) const
    {
      if ( !circ )
      {
        return g;
      }
      else
      {
        if ( circ->filter_cache.find( &g ) == circ->filter_cache.end() )
        {
          circ->filter_cache[&g] = new filtered_gate( g, circ->filter );
        }
        return *circ->filter_cache.find( &g )->second;
      }
    }

  private:
    subcircuit* circ;
  };
  /** @endcond */

  /** @cond */
  struct const_filter_circuit
  {
    typedef const gate& result_type;

    const_filter_circuit() : circ( 0 ) {}
    explicit const_filter_circuit( const subcircuit& circ ) : circ( &circ ) {}

    const gate& operator()( const gate& g ) const
    {
      if ( !circ )
      {
        return g;
      }
      else
      {
        // NOTE Works, but avoiding the const_cast would be better
        if ( circ->filter_cache.find( const_cast<gate*>( &g ) ) == circ->filter_cache.end() )
        {
          const_cast<subcircuit*>( circ )->filter_cache[const_cast<gate*>( &g )] = new filtered_gate( const_cast<gate&>( g ), const_cast<subcircuit*>( circ )->filter );
        }
        return *circ->filter_cache.find( const_cast<gate*>( &g ) )->second;
      }
    }

  private:
    const subcircuit* circ;
  };
  /** @endcond */

  /**
   * @brief Main circuit class
   *
   * This class represents a circuit and can be used generically for standard circuits and sub circuits.
   *
   * @author RevKit
   * @since  1.0
   */
  class circuit {
  public:
    /**
     * @brief Default constructor
     *
     * This constructor initializes a standard_circuit with 0 lines, also called an empty circuit.
     * Empty circuits are usually used as parameters for parsing functions, optimization algorithms, etc.
     *
     * @author RevKit
     * @since  1.0
     */
    circuit() {}
    std::map<unsigned, std::string> lines_to_inputs;
    std::vector<std::string> workers;
    std::vector<std::string> anc_zz;
    std::vector<std::string> anc_zg;
    std::vector<std::string> anc_11;
    std::vector<std::string> anc_1g;

    /**
     * @brief Cast Constructor for a standard_circuit
     *
     * With this constructor the standard_circuit constructor is automatically converted to 
     * a circuit, e.g. by calling
     *
     * @code
     * circuit circ( 3 );
     * @endcode
     *
     * a circuit with 3 lines is created.
     *
     * @param std_circ standard_circuit implementation
     *
     * @author RevKit
     * @since  1.0
     */
    circuit( const standard_circuit& std_circ ) : circ( std_circ ) {}

/**
     * @brief Cast Constructor for a standard_circuit
     *
     * With this constructor the standard_circuit constructor is automatically converted to 
     * a circuit, e.g. by calling
     *
     * @code
     * circuit circ( 3 );
     * @endcode
     *
     * a circuit with 3 lines is created.
     *
     * @param std_circ standard_circuit implementation
     *
     * @author RevKit
     * @since  1.0
     */
    circuit( standard_circuit& std_circ ) : circ( std_circ ) {}
    
    /**
     * @brief Cast Constructor for a subcircuit
     *
     * This constructor is used, so that subcircuits are detected as circuits in
     * algorithms and can passed as circuit parameter to other functions and
     * algorithms.
     *
     * @param sub_circ subcircuit implementation
     *
     * @author RevKit
     * @since  1.0
     */
    circuit( const subcircuit& sub_circ ) : circ( sub_circ ) {}
    
    /**
     * @brief Cast Constructor for a subcircuit
     *
     * This constructor is used, so that subcircuits are detected as circuits in
     * algorithms and can passed as circuit parameter to other functions and
     * algorithms.
     *
     * @param sub_circ subcircuit implementation
     *
     * @author RevKit
     * @since  1.0
     */
    circuit( subcircuit& sub_circ ) : circ( sub_circ ) {}

    /**
     * @brief Copy Constructor
     *
     * This constructor is used by some algorithms, but should not be used directly.
     *
     * It copies the underlying circuit, but it does not
     * copy the signals, so that this information gets lost.
     *
     * @param other Circuit to be copied
     *
     * @author RevKit
     * @since  1.0
     */
    circuit( const circuit& other ) : circ( other.circ ) {}

    /**
     * @brief Mutable iterator for accessing the gates in a circuit
     */
    typedef boost::transform_iterator<filter_circuit, boost::indirect_iterator<std::vector<boost::shared_ptr<gate> >::iterator> > iterator;

    /**
     * @brief Constant iterator for accessing the gates in a circuit
     */
    typedef boost::transform_iterator<const_filter_circuit, boost::indirect_iterator<std::vector<boost::shared_ptr<gate> >::const_iterator> > const_iterator;

    /**
     * @brief Mutable reverse iterator for accessing the gates in a circuit
     */
    typedef boost::transform_iterator<filter_circuit, boost::indirect_iterator<std::vector<boost::shared_ptr<gate> >::reverse_iterator> > reverse_iterator;
    /**
     * @brief Constant reverse iterator for accessing the gates in a circuit
     */
    typedef boost::transform_iterator<const_filter_circuit, boost::indirect_iterator<std::vector<boost::shared_ptr<gate> >::const_reverse_iterator> > const_reverse_iterator;

    /**
     * @brief Returns the number of gates
     *
     * This method returns the number of gates in the circuit.
     * 
     * @return Number of gates
     *
     * @author RevKit
     * @since  1.0
     */
    unsigned num_gates() const;

    /**
     * @brief Sets the number of line
     *
     * This method sets the number of lines of the circuit.
     *
     * Changing this number will not affect the data in the gates.
     * For example: If there is a gate with a control on line 3,
     * and the number of lines is reduced to 2 in the circuit, then
     * the control is still on line 3 but not visible in this circuit.
     *
     * So, changing the lines after already adding gates can lead
     * to invalid gates.
     *
     * @param lines Number of lines
     *
     * @author RevKit
     * @since  1.0
     */
    void set_lines( unsigned lines );

    /**
     * @brief Returns the number of lines
     *
     * This method returns the number of lines.
     *
     * @return Number of lines
     *
     * @author RevKit
     * @since  1.0
     */
    unsigned lines() const;

    /**
     * @brief Constant begin iterator pointing to gates
     *
     * @return Constant begin iterator
     *
     * @author RevKit
     * @since  1.0
     */
    const_iterator begin() const;
    
    /**
     * @brief Constant end iterator pointing to gates
     *
     * @return Constant end iterator
     *
     * @author RevKit
     * @since  1.0
     */
    const_iterator end() const;
    
    /**
     * @brief Mutable begin iterator pointing to gates
     *
     * @return Mutable begin iterator
     *
     * @author RevKit
     * @since  1.0
     */
    iterator begin();
    
    /**
     * @brief Mutable end iterator pointing to gates
     *
     * @return Mutable end iterator
     *
     * @author RevKit
     * @since  1.0
     */
    iterator end();

    /**
     * @brief Constant begin reverse iterator pointing to gates
     *
     * @return Constant begin reverse iterator
     *
     * @author RevKit
     * @since  1.0
     */
    const_reverse_iterator rbegin() const;
    
    /**
     * @brief Constant end reverse iterator pointing to gates
     *
     * @return Constant end reverse iterator
     *
     * @author RevKit
     * @since  1.0
     */
    const_reverse_iterator rend() const;
    
    /**
     * @brief Mutable begin reverse iterator pointing to gates
     *
     * @return Mutable begin reverse iterator
     *
     * @author RevKit
     * @since  1.0
     */
    reverse_iterator rbegin();
    
    /**
     * @brief Mutable end reverse iterator pointing to gates
     *
     * @return Mutable end reverse iterator
     *
     * @author RevKit
     * @since  1.0
     */
    reverse_iterator rend();

    /**
     * @brief Random access operator for access to gates by index
     *
     * @param index Index of the gate, starting from 0
     * @return constant access to the \p index gate in the circuit
     *
     * @author RevKit
     * @since  1.1
     */
    const gate& operator[]( unsigned index ) const;

    /**
     * @brief Random access operator for access to gates by index
     *
     * @param index Index of the gate, starting from 0
     * @return mutable access to the \p index gate in the circuit
     *
     * @author RevKit
     * @since  1.1
     */
    gate& operator[]( unsigned index );

    /**
     * @brief Inserts a gate at the end of the circuit
     *
     * This method inserts a gate at the end of the circuit.
     *
     * @return Reference to the newly created empty gate
     *
     * @author RevKit
     * @since  1.0
     */
    gate& append_gate();
    
    /**
     * @brief Inserts a gate at the beginning of the circuit
     *
     * This method inserts a gate at the beginning of the circuit.
     *
     * @return Reference to the newly created empty gate
     *
     * @author RevKit
     * @since  1.0
     */
    gate& prepend_gate();
    
    /**
     * @brief Inserts a gate into the circuit
     *
     * This method inserts a gate at an arbitrary position in the circuit
     *
     * @param pos  Position where to insert the gate
     *
     * @return Reference to the newly created empty gate
     *
     * @author RevKit
     * @since  1.0
     */
    gate& insert_gate( unsigned pos );

    /**
     * @brief Removes a gate at a given index
     *
     * If the index is not valid, no gate is removed.
     *
     * @param pos  Index
     *
     * @author RevKit
     * @since  1.0
     */
    void remove_gate_at( unsigned pos );


    /**
    * @brief Allows integration of bitvectors into circuit signals
    *
    * 
    * @param inputs input name
    * @author Adam Holmes
    * @since 1.3
    */
    
    void initialize_inputs( int* input );
    void remove_input( int* input );
    void remove_worker( int* input );
    void initialize_worker( int* id, std::string* name  ); 
    void initialize_ancilla( int* id, std::string* input, int classifier );
    void print_signal( std::string* name, int flag );





    /**
     * @brief Sets the input names of the lines in a circuit
     *
     * This method sets the input names of the lines in a circuit.
     * This is useful for functions when writing them to a file,
     * printing them, or creating images.
     *
     * @param inputs Input names
     *
     * @author RevKit
     * @since  1.0
     */
    void set_inputs( const std::vector<std::string>& inputs );
//    void set_workers( const std::vector<std::string>& inputs );
//    void set_ancilla_zg( const std::vector<std::string>& inputs );
//    void set_ancilla_zz( const std::vector<std::string>& inputs );
//    void set_ancilla_1g( const std::vector<std::string>& inputs );
//    void set_ancilla_11( const std::vector<std::string>& inputs );


    /* 
    * @brief Adds new garbage signals to the circuit
    *
    * This method creates ancilla signals by declaring them as garbage signals.
    * 
    *
    
    void zero_to_garbage( std::vector<int>& inputs );
    void one_to_garbage( std::vector<int>& inputs );
    void zero_to_zero( std::vector<int>& inputs );
    void one_to_one( std::vector<int>& inputs );
    */



    
    /**
     * @brief Returns the input names of the lines in a circuit
     *
     * This method returns the input names of the lines in a circuit.
     * This is useful for functions when writing them to a file,
     * printing them, or creating images.
     *
     * @return Input names
     *
     * @author RevKit
     * @since  1.0
     */
    const std::vector<std::string>& inputs() const;
//    const std::vector<std::string>& get_workers() const;
//    const std::vector<std::string>& anc_zg() const;
//    const std::vector<std::string>& anc_zz() const;
//    const std::vector<std::string>& anc_1g() const;
//    const std::vector<std::string>& anc_11() const;
    
    /**
     * @brief Sets the output names of the lines in a circuit
     *
     * This method sets the output names of the lines in a circuit.
     * This is useful for functions when writing them to a file,
     * printing them, or creating images.
     *
     * @param outputs Output names
     *
     * @author RevKit
     * @since  1.0
     */
    void set_outputs( const std::vector<std::string>& outputs );
    
    /**
     * @brief Returns the output names of the lines in a circuit
     *
     * This method returns the output names of the lines in a circuit.
     * This is useful for functions when writing them to a file,
     * printing them, or creating images.
     *
     * @return Output names
     *
     * @author RevKit
     * @since  1.0
     */
    const std::vector<std::string>& outputs() const;
    
    /**
     * @brief Sets the constant input line specifications
     *
     * This method sets the constant input line specification.
     *
     * Lines are by default not constant. If less values are given
     * than lines exist, the last ones will be not constant. If more
     * values are given than lines exist, they will be truncated.
     *
     * @sa constant
     *
     * @param constants Constant Lines
     *
     * @author RevKit
     * @since  1.0
     */
    void set_constants( const std::vector<constant>& constants );
    
    /**
     * @brief Returns the constant input line specification
     *
     * This method returns the constant input line specification.
     *
     * @return Constant input line specification
     *
     * @author RevKit
     * @since  1.0
     */
    const std::vector<constant>& constants() const;
    
    /**
     * @brief Sets whether outputs are garbage or not
     *
     * If an output is garbage it means, that the resulting
     * output value is not necessary for the function.
     *
     * Lines are by default not garbage. If less values are given
     * than lines exist, the last ones will be not garbage. If more
     * values are given than lines exist, they will be truncated.
     *
     * @param garbage Garbage line specification
     *
     * @author RevKit
     * @since  1.0
     */
    void set_garbage( const std::vector<bool>& garbage );
    
    /**
     * @brief Returns whether outputs are garbage or not
     *
     * This method returns the garbage line specification.
     *
     * @return Garbage output line specification
     *
     * @author RevKit
     * @since  1.0
     */
    const std::vector<bool>& garbage() const;
    
    /**
     * @brief Sets a name of the circuit
     *
     * Sets a name for the circuit which is empty
     * initially.
     *
     * @param name Name
     *
     * @author RevKit
     * @since  1.0
     */
    void set_circuit_name( const std::string& name );
    
    /**
     * @brief Returns the name of the circuit
     *
     * Returns the name of the circuit which is empty
     * initially.
     *
     * @return Name of the circuit
     *
     * @author RevKit
     * @since  1.0
     */
    const std::string& circuit_name() const;

    /** 
     * @brief Constant access to the input buses
     * 
     * This method gives constant access to the input
     * buses.
     * 
     * @return Input bus collection
     *
     * @author RevKit
     * @since  1.1
     */
    const bus_collection& inputbuses() const;

    /** 
     * @brief Mutable access to the input buses
     * 
     * This method gives mutable access to the input
     * buses.
     * 
     * @return Input bus collection
     *
     * @author RevKit
     * @since  1.1
     */
    bus_collection& inputbuses();

    /** 
     * @brief Constant access to the output buses
     * 
     * This method gives constant access to the output
     * buses.
     * 
     * @return Output bus collection
     *
     * @author RevKit
     * @since  1.1
     */
    const bus_collection& outputbuses() const;

    /** 
     * @brief Mutable access to the output buses
     * 
     * This method gives mutable access to the output
     * buses.
     * 
     * @return Output bus collection
     *
     * @author RevKit
     * @since  1.1
     */
    bus_collection& outputbuses();

    /** 
     * @brief Constant access to the state signals
     * 
     * This method gives constant access to the state
     * signals.
     * 
     * @return State signal collection
     *
     * @author RevKit
     * @since  1.1
     */
    const bus_collection& statesignals() const;

    /** 
     * @brief Mutable access to the state signals
     * 
     * This method gives mutable access to the state
     * signals.
     * 
     * @return State signal collection
     *
     * @author RevKit
     * @since  1.1
     */
    bus_collection& statesignals();

    /**
     * @brief Returns whether the circuit is a sub-circuit or not
     *
     * Both standard_circuit and subcircuit are used in the context as a circuit
     * in other algorithms. To determine what kind of circuit it is, this method
     * returns \p true if the circuit is a sub-circuit, \p false otherwise.
     *
     * @return true, if circuit is a sub-circuit, \p false otherwise
     *
     * @author RevKit
     * @since  1.0
     */
    bool is_subcircuit() const;

    /**
     * @brief Returns the filter of a sub-circuit
     *
     * This method returns a pair <i>(l, f)</i>.
     * In case the circuit is sub-circuit restricted on the lines,
     * \em l is the original number of lines of the base circuit
     * and \em f is the filter, i.e. a vector of lines which are
     * used for the sub-circuit. In other cases \em s is always 0
     * and \em f is empty.
     *
     * @return Pair of information about the sub-circuit's base or \em empty pair
     *
     * @author RevKit
     * @since  1.0
     */
    std::pair<unsigned, std::vector<unsigned> > filter() const;

    /**
     * @brief Returns the offset of the circuit (sub-circuit)
     *
     * For a standard_circuit, the offset is always 0, but for a
     * sub-circuit, the offset is the index of the starting gate
     * in respect to its base circuit.
     *
     * @return Offset of the circuit
     *
     * @author RevKit
     * @since  1.0
     */
    unsigned offset() const;

    /**
     * @brief Adds a module to the circuit
     *
     * This function adds a module to the circuit. It does
     * not create a gate calling the module, but the module
     * itself as a reference for further use, e.g. with append_module.
     *
     * This method uses smart pointer to a circuit wich already
     * exists and is managed by another object. If it cannot
     * be assured that the module is saved, the method
     * add_module(const std::string&, const circuit&) should be used.
     *
     * @param name Name of the module
     * @param module Reference to an existing module
     *
     * @author RevKit
     * @since  1.1
     */
    void add_module( const std::string& name, const boost::shared_ptr<circuit>& module );
    
    /** 
     * @brief Adds a module to the circuit
     *
     * This function adds a module to the circuit. It does
     * not create a gate calling the module, but the module
     * itself as a reference for further use, e.g. with append_module.
     *
     * In this method the module is copied first and thus assured
     * that is managed by this circuit.
     * 
     * @param name Name of the module
     * @param module Module to be copied
     *
     * @author RevKit
     * @since  1.1
     */
    void add_module( const std::string& name, const circuit& module );
    
    /** 
     * @brief Returns all modules in the circuit
     * 
     * This method returns a map of all modules, whereby the
     * keys are the names of the modules.
     * 
     * @return Map of modules
     *
     * @author RevKit
     * @since  1.1
     */
    const std::map<std::string, boost::shared_ptr<circuit> >& modules() const;

    /** 
     * @brief Returns the annotation for one gate and one key
     *
     * This method returns the value for one particular annotation for
     * a given gate. If no annotation with that key exists, the a default
     * value is given. 
     *
     * @param g Gate
     * @param key Key of the annotation
     * @param default_value Default value, in case the key does not exist
     * 
     * @return Value of the annotation or the default value
     *
     * @author RevKit
     * @since  1.1
     */
    const std::string& annotation( const gate& g, const std::string& key, const std::string& default_value = std::string() ) const;

    /** 
     * @brief Returns all annotations for a given gate
     *
     * This method returns all annotations for a given gate. For the
     * purpose of efficiency, this method returns an optional data type
     * containing the property map. So, first check whether there are
     * items by assierting the optional, and then go through the map
     * by dereferencing the optional:
     * @code
     * boost::optional<const std::map<std::string, std::string>& > annotations = circ.annotations( g );
     *
     * if ( annotations )
     * {
     *   // annotations exists
     *   typedef std::pair<std::string, std::string> pair_t;
     *   foreach ( const pair_t& p, *annotations )
     *   {
     *     const std::string& key = p.first;
     *     const std::string& value = p.second;
     *     // do something with key and value
     *   }
     * }
     * @endcode
     * 
     * @param g Gate
     * 
     * @return Map of annotations encapsulated in an optional
     *
     * @author RevKit
     * @since  1.1
     */
    boost::optional<const std::map<std::string, std::string>& > annotations( const gate& g ) const;

    /** 
     * @brief Annotates a gate
     * 
     * With this method a gate can be annotated using a key and a value.
     * If there is an annotation with the same key, it will be overwritten.
     *
     * @param g Gate
     * @param key Key of the annotation
     * @param value Value of the annotation
     *
     * @author RevKit
     * @since  1.1
     */
    void annotate( const gate& g, const std::string& key, const std::string& value );

    // SIGNALS
    /**
     * @brief Signal which is emitted after adding a gate
     *
     * The gate is always empty, since when adding a gate to the
     * circuit an empty gate is returned as reference and then
     * further processed by functions such as append_toffoli.
     */
    boost::signal<void(gate&)> gate_added;

    /** @cond */
    operator circuit_variant&()
    {
      return circ;
    }

    operator const circuit_variant&() const
    {
      return circ;
    }
    /** @endcond */
  private:
    /** @cond */
    circuit_variant circ;
    std::map<std::string, boost::shared_ptr<circuit> > _modules;
    /** @endcond */
  };


  extern circuit global_circuit;
  extern bool LLVM_IR;
  extern int registerCount;

  class qint{
    public:
        static circuit circ;
        std::vector<qint*> reg;
        std::string name;
    	int id;
        qint();
        qint(int num);
    	qint operator[] (const int index);
  };

  class qbit : public qint{
    public:
        qbit();
        qbit(bool flag);
        qbit(int num);
        qbit(int length, int width);
  };

  class zero_to_garbage : public qint{
    public:
    	zero_to_garbage();
    	zero_to_garbage(int num);
    	zero_to_garbage(int length, int width);
  };

  class zero_to_zero : public qint{
    public:
        zero_to_zero();
        zero_to_zero(int num);
        zero_to_zero(int length, int width);
  };

  class one_to_garbage : public qint{
    public:
        one_to_garbage();
        one_to_garbage(int num);
        one_to_garbage(int length, int width);
  };


  class one_to_one : public qint{
    public:
        one_to_one();
        one_to_one(int num);
        one_to_one(int length, int width);
  };



}

#endif /* CIRCUIT_HPP */
