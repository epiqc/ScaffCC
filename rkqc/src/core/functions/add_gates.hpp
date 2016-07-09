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
 * @file add_gates.hpp
 *
 * @brief Adding typical gates to a circuit
 */

#ifndef ADD_GATES_HPP
#define ADD_GATES_HPP

#include <core/circuit.hpp>

namespace revkit
{

  /**
   * @brief Helper class for adding lines in an easier way
   *
   * This class should not be used stand alone but just with the add_\em gate methods
   * designed for this purpose. See also \ref sub_add_gates.
   *
   * @author RevKit
   * @since  1.0
   *
   * @sa \ref sub_add_gates
   */
  class target_line_adder
  {
  public:
    /**
     * @brief Default constructor
     *
     * @param gate Gate, to which target lines should be added
     *
     * @sa \ref sub_add_gates
     *
     * @author RevKit
     * @since  1.0
     */
    explicit target_line_adder( gate* gate );

    /**
     * @brief Add one target line
     *
     * @param  l1 First target line
     *
     * @return A smart pointer to the gate
     *
     * @sa \ref sub_add_gates
     *
     * @author RevKit
     * @since  1.0
     */
    gate& operator()( const gate::line& l1 );

    /**
     * @brief Add two target lines
     *
     * @param  l1 First target line
     * @param  l2 Second target line
     *
     * @return A smart pointer to the gate
     *
     * @sa \ref sub_add_gates
     *
     * @author RevKit
     * @since  1.0
     */
    gate& operator()( const gate::line& l1, const gate::line& l2 );

  private:
    gate* g;
  };

  /**
   * @brief Helper class for adding lines in an easier way
   *
   * This class should not be used stand alone but just with the add_\em gate methods
   * designed for this purpose. See also \ref sub_add_gates.
   *
   * @author RevKit
   * @since  1.0
   *
   * @sa \ref sub_add_gates
   */
  class control_line_adder
  {
  public:
    /**
     * @brief Default constructor
     *
     * @param g Gate, to which control lines should be added
     *
     * @sa \ref sub_add_gates
     *
     * @author RevKit
     * @since  1.0
     */
    explicit control_line_adder( gate& g );

    /**
     * @brief Add no control line
     *
     * @return A target_line_adder
     *
     * @sa \ref sub_add_gates
     *
     * @author RevKit
     * @since  1.0
     */
    target_line_adder operator()();

    /**
     * @brief Add one control line
     *
     * @param  l1 First control line
     *
     * @return A target_line_adder
     *
     * @sa \ref sub_add_gates
     *
     * @author RevKit
     * @since  1.0
     */
    target_line_adder operator()( const gate::line& l1 );

    /**
     * @brief Add two control lines
     *
     * @param  l1 First control line
     * @param  l2 Second control line
     *
     * @return A target_line_adder
     *
     * @sa \ref sub_add_gates
     *
     * @author RevKit
     * @since  1.0
     */
    target_line_adder operator()( const gate::line& l1, const gate::line& l2 );

    /**
     * @brief Add three control lines
     *
     * @param  l1 First control line
     * @param  l2 Second control line
     * @param  l3 Second control line
     *
     * @return A target_line_adder
     *
     * @sa \ref sub_add_gates
     *
     * @author RevKit
     * @since  1.0
     */
    target_line_adder operator()( const gate::line& l1, const gate::line& l2, const gate::line& l3 );

    /**
     * @brief Add four control lines
     *
     * @param  l1 First control line
     * @param  l2 Second control line
     * @param  l3 Second control line
     * @param  l4 Fourth control line
     *
     * @return A target_line_adder
     *
     * @sa \ref sub_add_gates
     *
     * @author RevKit
     * @since  1.0
     */
    target_line_adder operator()( const gate::line& l1, const gate::line& l2, const gate::line& l3, const gate::line& l4 );

    /**
     * @brief Add five control lines
     *
     * @param  l1 First control line
     * @param  l2 Second control line
     * @param  l3 Second control line
     * @param  l4 Fourth control line
     * @param  l5 Fifth control line
     *
     * @return A target_line_adder
     *
     * @sa \ref sub_add_gates
     *
     * @author RevKit
     * @since  1.0
     */
    target_line_adder operator()( const gate::line& l1, const gate::line& l2, const gate::line& l3, const gate::line& l4, const gate::line& l5 );

    /**
     * @brief Add six control lines
     *
     * @param  l1 First control line
     * @param  l2 Second control line
     * @param  l3 Second control line
     * @param  l4 Fourth control line
     * @param  l5 Fifth control line
     * @param  l6 Sixth control line
     *
     * @return A target_line_adder
     *
     * @sa \ref sub_add_gates
     *
     * @author RevKit
     * @since  1.0
     */
    target_line_adder operator()( const gate::line& l1, const gate::line& l2, const gate::line& l3, const gate::line& l4, const gate::line& l5, const gate::line& l6 );

    /**
     * @brief Add seven control lines
     *
     * @param  l1 First control line
     * @param  l2 Second control line
     * @param  l3 Second control line
     * @param  l4 Fourth control line
     * @param  l5 Fifth control line
     * @param  l6 Sixth control line
     * @param  l7 Seventh control line
     *
     * @return A target_line_adder
     *
     * @sa \ref sub_add_gates
     *
     * @author RevKit
     * @since  1.0
     */
    target_line_adder operator()( const gate::line& l1, const gate::line& l2, const gate::line& l3, const gate::line& l4, const gate::line& l5, const gate::line& l6, const gate::line& l7 );

    /**
     * @brief Add eight control lines
     *
     * @param  l1 First control line
     * @param  l2 Second control line
     * @param  l3 Second control line
     * @param  l4 Fourth control line
     * @param  l5 Fifth control line
     * @param  l6 Sixth control line
     * @param  l7 Seventh control line
     * @param  l8 Eighth control line
     *
     * @return A target_line_adder
     *
     * @sa \ref sub_add_gates
     *
     * @author RevKit
     * @since  1.0
     */
    target_line_adder operator()( const gate::line& l1, const gate::line& l2, const gate::line& l3, const gate::line& l4, const gate::line& l5, const gate::line& l6, const gate::line& l7, const gate::line& l8 );

    /**
     * @brief Add nine control lines
     *
     * @param  l1 First control line
     * @param  l2 Second control line
     * @param  l3 Second control line
     * @param  l4 Fourth control line
     * @param  l5 Fifth control line
     * @param  l6 Sixth control line
     * @param  l7 Seventh control line
     * @param  l8 Eighth control line
     * @param  l9 Ninth control line
     *
     * @return A target_line_adder
     *
     * @sa \ref sub_add_gates
     *
     * @author RevKit
     * @since  1.0
     */
    target_line_adder operator()( const gate::line& l1, const gate::line& l2, const gate::line& l3, const gate::line& l4, const gate::line& l5, const gate::line& l6, const gate::line& l7, const gate::line& l8, const gate::line& l9 );

  private:
    gate* g;
  };

  /**
   * @brief Helper function for appending a \b Toffoli gate
   *
   * @param circ     Circuit
   * @param controls Control Lines
   * @param target   Target Line
   *
   * @return Gate reference
   *
   * @author RevKit
   * @since  1.0
   */
  gate& append_toffoli( circuit& circ, const gate::line_container& controls, const gate::line& target );

  /**
   * @brief Helper function for appending a \b Fredkin gate
   *
   * @param circ     Circuit
   * @param controls Control Lines
   * @param target1  Target Line 1
   * @param target2  Target Line 2
   *
   * @return Gate reference
   *
   * @author RevKit
   * @since  1.0
   */
  gate& append_fredkin( circuit& circ, const gate::line_container& controls, const gate::line& target1, const gate::line& target2 );

  /**
   * @brief Helper function for appending a \b Peres gate
   *
   * @param circ    Circuit
   * @param control Control Line
   * @param target1 Target Line 1
   * @param target2 Target Line 2
   *
   * @return Gate reference
   *
   * @author RevKit
   * @since  1.0
   */
  gate& append_peres( circuit& circ, const gate::line& control, const gate::line& target1, const gate::line& target2 );

  /**
   * @brief Helper function for appending a \b CNOT gate
   *
   * @param circ    Circuit
   * @param control Control Line
   * @param target  Target Line
   *
   * @return Gate reference
   *
   * @author RevKit
   * @since  1.0
   */
//  gate& append_cnot( circuit& circ, const gate::line& control, const gate::line& target );
  void append_cnot( circuit& circ, const gate::line& control, const gate::line& target );

  /**
   * @brief Helper function for appending a \b V gate
   *
   * @param circ    Circuit
   * @param control Control Line
   * @param target  Target Line
   *
   * @return Gate reference
   *
   * @author RevKit
   * @since  1.0
   */
  gate& append_v( circuit& circ, const gate::line& control, const gate::line& target );

  /**
   * @brief Helper function for appending a \b V+ gate
   *
   * @param circ    Circuit
   * @param control Control Line
   * @param target  Target Line
   *
   * @return Gate reference
   *
   * @author RevKit
   * @since  1.0
   */
  gate& append_vplus( circuit& circ, const gate::line& control, const gate::line& target );

  /**
   * @brief Helper function for appending a \b NOT gate
   *
   * @param circ    Circuit
   * @param target  Target Line
   *
   * @return Gate reference
   *
   * @author RevKit
   * @since  1.0
   */
//  gate& append_not( circuit& circ, const gate::line& target );
  void append_not( circuit& circ, const gate::line& target );


  void NOT( qint target );
  void cnot( qint control, qint target );
  void toffoli( qint control1, qint control2, qint target );
  void toffoli( qint control1, qint control2, qint control3, qint target );

  void set_LLVM( bool setter );

/*--------------- CTQG Functions --------------------------*/

  void assign_value_of_int_to_a( qint a, int b, int size = 1 );
  void assign_value_of_b_to_a( qint a, qint b, int size = 1 );
  void assign_value_of_b_to_a( qint a, std::string binary_constant,  int size = 1 );
  void a_eq_a_plus_b( qint a, qint b, int size = 1);
  void a_swap_b( qint a, qint b, int size = 1);
  void a_eq_a_minus_b( qint a, qint b, int size = 1);
  void a_eq_a_plus_b_times_c( qint a, qint b, qint c, int size = 1);
  void a_eq_a_minus_b_times_c( qint a, qint b, qint c, int size = 1);


  /**
   * @brief Helper function for appending a module gate
   *
   * @param circ        Circuit
   * @param module_name Name of the module (already added to the circuit)
   * @param controls    Control Lines
   * @param targets     Target Lines (has to be a ordered vector for the mapping to the module lines)
   *
   * @return Gate reference
   *
   * @author RevKit
   * @since  1.1
   */
  gate& append_module( circuit& circ, const std::string& module_name, const gate::line_container& controls, const std::vector<unsigned>& targets );

  /**
   * @brief Helper function for appending a generic gate using the control_line_adder
   *
   * @param circ Circuit
   * @param tag  Gate type tag
   *
   * @return A control_line_adder
   *
   * @sa \ref sub_add_gates
   * @sa \ref sub_target_tags
   *
   * @author RevKit
   * @since  1.0
   */
  control_line_adder append_gate( circuit& circ, const boost::any& tag );

  /**
   * @brief Helper function for appending a \b Toffoli gate using the control_line_adder
   *
   * @param circ Circuit
   *
   * @return A control_line_adder
   *
   * @sa \ref sub_add_gates
   *
   * @author RevKit
   * @since  1.0
   */
  control_line_adder append_toffoli( circuit& circ );

  /**
   * @brief Helper function for appending a \b Fredkin gate using the control_line_adder
   *
   * @param circ Circuit
   *
   * @return A control_line_adder
   *
   * @sa \ref sub_add_gates
   *
   * @author RevKit
   * @since  1.0
   */
  control_line_adder append_fredkin( circuit& circ );

  /**
   * @brief Helper function for prepending a \b Toffoli gate
   *
   * @param circ     Circuit
   * @param controls Control Lines
   * @param target   Target Line
   *
   * @return Gate reference
   *
   * @author RevKit
   * @since  1.0
   */
  gate& prepend_toffoli( circuit& circ, const gate::line_container& controls, const gate::line& target );

  /**
   * @brief Helper function for prepending a \b Fredkin gate
   *
   * @param circ     Circuit
   * @param controls Control Lines
   * @param target1  Target Line 1
   * @param target2  Target Line 2
   *
   * @return Gate reference
   *
   * @author RevKit
   * @since  1.0
   */
  gate& prepend_fredkin( circuit& circ, const gate::line_container& controls, const gate::line& target1, const gate::line& target2 );

  /**
   * @brief Helper function for prepending a \b Peres gate
   *
   * @param circ    Circuit
   * @param control Control Line
   * @param target1 Target Line 1
   * @param target2 Target Line 2
   *
   * @return Gate reference
   *
   * @author RevKit
   * @since  1.0
   */
  gate& prepend_peres( circuit& circ, const gate::line& control, const gate::line& target1, const gate::line& target2 );

  /**
   * @brief Helper function for prepending a \b CNOT gate
   *
   * @param circ    Circuit
   * @param control Control Line
   * @param target  Target Line
   *
   * @return Gate reference
   *
   * @author RevKit
   * @since  1.0
   */
//  gate& prepend_cnot( circuit& circ, const gate::line& control, const gate::line& target );
  void prepend_cnot( circuit& circ, const gate::line& control, const gate::line& target );

  /**
   * @brief Helper function for prepending a \b V gate
   *
   * @param circ    Circuit
   * @param control Control Line
   * @param target  Target Line
   *
   * @return Gate reference
   *
   * @author RevKit
   * @since  1.0
   */
  gate& prepend_v( circuit& circ, const gate::line& control, const gate::line& target );

  /**
   * @brief Helper function for prepending a \b V+ gate
   *
   * @param circ    Circuit
   * @param control Control Line
   * @param target  Target Line
   *
   * @return Gate reference
   *
   * @author RevKit
   * @since  1.0
   */
  gate& prepend_vplus( circuit& circ, const gate::line& control, const gate::line& target );

  /**
   * @brief Helper function for prepending a \b NOT gate
   *
   * @param circ    Circuit
   * @param target  Target Line
   *
   * @return Gate reference
   *
   * @author RevKit
   * @since  1.0
   */
//  gate& prepend_not( circuit& circ, const gate::line& target );
  void prepend_not( circuit& circ, const gate::line& target );

  /**
   * @brief Helper function for prepending a module gate
   *
   * @param circ        Circuit
   * @param module_name Name of the module (already added to the circuit)
   * @param controls    Control Lines
   * @param targets     Target Lines (has to be a ordered vector for the mapping to the module lines)
   *
   * @return Gate reference
   *
   * @author RevKit
   * @since  1.1
   */
  gate& prepend_module( circuit& circ, const std::string& module_name, const gate::line_container& controls, const std::vector<unsigned>& targets );

  /**
   * @brief Helper function for prepending a generic gate using the control_line_adder
   *
   * @param circ Circuit
   * @param tag  Gate type tag
   *
   * @return A control_line_adder
   *
   * @sa \ref sub_add_gates
   * @sa \ref sub_target_tags
   *
   * @author RevKit
   * @since  1.0
   */
  control_line_adder prepend_gate( circuit& circ, const boost::any& tag );

  /**
   * @brief Helper function for prepending a \b Toffoli gate using the control_line_adder
   *
   * @param circ Circuit
   *
   * @return A control_line_adder
   *
   * @sa \ref sub_add_gates
   *
   * @author RevKit
   * @since  1.0
   */
  control_line_adder prepend_toffoli( circuit& circ );

  /**
   * @brief Helper function for prepending a \b Fredkin gate using the control_line_adder
   *
   * @param circ Circuit
   *
   * @return A control_line_adder
   *
   * @sa \ref sub_add_gates
   *
   * @author RevKit
   * @since  1.0
   */
  control_line_adder prepend_fredkin( circuit& circ );




  /**
   * @brief Helper function for inserting a \b Toffoli gate
   *
   * @param circ     Circuit
   * @param n        Index to insert the gate
   * @param controls Control Lines
   * @param target   Target Line
   *
   * @return Gate reference
   *
   * @author RevKit
   * @since  1.0
   */
  gate& insert_toffoli( circuit& circ, unsigned n, const gate::line_container& controls, const gate::line& target );

  /**
   * @brief Helper function for inserting a \b Fredkin gate
   *
   * @param circ     Circuit
   * @param n        Index to insert the gate
   * @param controls Control Lines
   * @param target1  Target Line 1
   * @param target2  Target Line 2
   *
   * @return Gate reference
   *
   * @author RevKit
   * @since  1.0
   */
  gate& insert_fredkin( circuit& circ, unsigned n, const gate::line_container& controls, const gate::line& target1, const gate::line& target2 );

  /**
   * @brief Helper function for inserting a \b Peres gate
   *
   * @param circ    Circuit
   * @param n       Index to insert the gate
   * @param control Control Line
   * @param target1 Target Line 1
   * @param target2 Target Line 2
   *
   * @return Gate reference
   *
   * @author RevKit
   * @since  1.0
   */
  gate& insert_peres( circuit& circ, unsigned n, const gate::line& control, const gate::line& target1, const gate::line& target2 );

  /**
   * @brief Helper function for inserting a \b CNOT gate
   *
   * @param circ    Circuit
   * @param n       Index to insert the gate
   * @param control Control Line
   * @param target  Target Line
   *
   * @return Gate reference
   *
   * @author RevKit
   * @since  1.0
   */
//  gate& insert_cnot( circuit& circ, unsigned n, const gate::line& control, const gate::line& target );
  void insert_cnot( circuit& circ, unsigned n, const gate::line& control, const gate::line& target );

  /**
   * @brief Helper function for inserting a \b V gate
   *
   * @param circ    Circuit
   * @param n       Index to insert the gate
   * @param control Control Line
   * @param target  Target Line
   *
   * @return Gate reference
   *
   * @author RevKit
   * @since  1.0
   */
  gate& insert_v( circuit& circ, unsigned n, const gate::line& control, const gate::line& target );

  /**
   * @brief Helper function for inserting a \b V+ gate
   *
   * @param circ    Circuit
   * @param n       Index to insert the gate
   * @param control Control Line
   * @param target  Target Line
   *
   * @return Gate reference
   *
   * @author RevKit
   * @since  1.0
   */
  gate& insert_vplus( circuit& circ, unsigned n, const gate::line& control, const gate::line& target );

  /**
   * @brief Helper function for inserting a \b NOT gate
   *
   * @param circ    Circuit
   * @param n       Index to insert the gate
   * @param target  Target Line
   *
   * @return Gate reference
   *
   * @author RevKit
   * @since  1.0
   */
//  gate& insert_not( circuit& circ, unsigned n, const gate::line& target );
  void insert_not( circuit& circ, unsigned n, const gate::line& target );

  /**
   * @brief Helper function for inserting a module gate
   *
   * @param circ        Circuit
   * @param n           Index to insert the gate
   * @param module_name Name of the module (already added to the circuit)
   * @param controls    Control Lines
   * @param targets     Target Lines (has to be a ordered vector for the mapping to the module lines)
   *
   * @return Gate reference
   *
   * @author RevKit
   * @since  1.1
   */
  gate& insert_module( circuit& circ, unsigned n, const std::string& module_name, const gate::line_container& controls, const std::vector<unsigned>& targets );

  /**
   * @brief Helper function for inserting a generic gate using the control_line_adder
   *
   * @param circ Circuit
   * @param n    Index to insert the gate
   * @param tag  Gate type tag
   *
   * @return A control_line_adder
   *
   * @sa \ref sub_add_gates
   * @sa \ref sub_target_tags
   *
   * @author RevKit
   * @since  1.0
   */
  control_line_adder insert_gate( circuit& circ, unsigned n, const boost::any& tag );

  /**
   * @brief Helper function for inserting a \b Toffoli gate using the control_line_adder
   *
   * @param circ Circuit
   * @param n    Index to insert the gate
   *
   * @return A control_line_adder
   *
   * @sa \ref sub_add_gates
   *
   * @author RevKit
   * @since  1.0
   */
  control_line_adder insert_toffoli( circuit& circ, unsigned n );

  /**
   * @brief Helper function for inserting a \b Fredkin gate using the control_line_adder
   *
   * @param circ Circuit
   * @param n    Index to insert the gate
   *
   * @return A control_line_adder
   *
   * @sa \ref sub_add_gates
   *
   * @author RevKit
   * @since  1.0
   */
  control_line_adder insert_fredkin( circuit& circ, unsigned n );


}

#endif /* ADD_GATES_HPP */
