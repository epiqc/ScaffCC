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
 * @file costs.hpp
 *
 * @brief Cost calculation for circuits
 */

#ifndef COSTS_HPP
#define COSTS_HPP

#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/variant.hpp>

#include <core/circuit.hpp>

namespace revkit
{

  /**
   * @brief Type for costs
   *
   * Costs is an unsigned long long, that is usually
   * at least a 64-bit integer, but depending on the
   * platform.
   *
   * @author RevKit
   * @since  1.2
   */
  typedef unsigned long long cost_t;

  /**
   * @brief Functor for cost function measured by the whole circuit
   *
   * @author RevKit
   * @since  1.0
   */
  typedef boost::function<cost_t( const circuit& circ )> costs_by_circuit_func;

  /**
   * @brief Functor for cost function measured by each gate
   *
   * @author RevKit
   * @since  1.0
   */
  typedef boost::function<cost_t( const gate& gate, unsigned lines )> costs_by_gate_func;

  /**
   * @brief Cost Function type
   *
   * It can either be a costs_by_circuit_func or a costs_by_gate_func
   *
   * @author RevKit
   * @since  1.0
   */
  typedef boost::variant<costs_by_circuit_func, costs_by_gate_func> cost_function;

  /**
   * @brief Calculates the gate costs
   *
   * This costs class is basically a wrapper around num_gates()
   * and is given for convenience use with other cost
   * functions.
   *
   * @sa \ref sub_cost_functions
   *
   * @author RevKit
   * @since  1.0
   */
  struct gate_costs
  {
    /**
     * @brief Returns the number of gates
     *
     * @param circ Circuit
     *
     * @return Number of gates
     *
     * @author RevKit
     * @since  1.0
     */
    cost_t operator()( const circuit& circ ) const;
  };

  /**
   * @brief Calculates the line costs
   *
   * This costs class is basically a wrapper around lines()
   * and is given for convenience use with other cost
   * functions.
   *
   * @sa \ref sub_cost_functions
   *
   * @author RevKit
   * @since  1.0
   */
  struct line_costs
  {
    /**
     * @brief Returns the number of lines
     *
     * @param circ Circuit
     *
     * @return Number of lines
     *
     * @author RevKit
     * @since  1.0
     */
    cost_t operator()( const circuit& circ ) const;
  };

  /**
   * @brief Calculates the quantum costs
   *
   * This costs class calculates the quantum costs.
   * Thereby known optimizations for gates with few control lines
   * or for gates with empty lines are considered.
   *
   * @sa \ref sub_cost_functions
   *
   * @author RevKit
   * @since  1.0
   */
  struct quantum_costs
  {
    /**
     * @brief Standard constructor
     *
     * Initializes default values
     *
     * @author RevKit
     * @since  1.1
     */
    quantum_costs();

    /**
     * @brief Returns the quantum costs for gate \p g
     *
     * @param g Gate
     * @param lines Number of lines in the circuit
     *
     * @return Quantum Costs for gate \p g
     *
     * @author RevKit
     * @since  1.0
     */
    cost_t operator()( const gate& g, unsigned lines ) const;

    /**
     * @brief Offset for control lines
     *
     * With this parameter the number of controls can be
     * modified without changing the gate. This enables
     * hypothetical costs calculations.
     *
     * Note, that the offset is \em adjusted if it leads
     * to impossible gates such as negative number of control
     * lines or too many control lines for the existing gates.
     *
     * The default value is \p 0
     *
     * @author RevKit
     * @since  1.1
     */
    int controls_offset;
  };

  /**
   * @brief Calculates the transistor costs
   *
   * This class calculates the so called transistor
   * costs for a gate. They are the number of
   * control lines multiplied by 8.
   *
   * @sa \ref sub_cost_functions
   *
   * @author RevKit
   * @since  1.0
   */
  struct transistor_costs
  {
    /**
     * @brief Returns the transistor costs for gate \p g
     *
     * @param g Gate
     * @param lines Number of lines in the circuit
     *
     * @return Transistor Costs for gate \p g
     *
     * @author RevKit
     * @since  1.0
     */
    cost_t operator()( const gate& g, unsigned lines ) const;
  };

  /**
   * @brief Calculates the costs of a circuit by a given cost function
   *
   * With this function the costs for a circuit can be calculated.
   * Thereby this functions is generic and calls a cost_function for determine the costs.
   * The costs function can either be derived from costs_by_circuit, whereby
   * the costs are calculated on base by the whole circuit or it can be
   * derived from costs_by_gate, whereby the costs are of each gate are calculated
   * and the sum is returned.
   *
   * @param circ Circuit
   * @param f Cost function
   *
   * @return The costs for the circuit in respect to the given cost function
   *
   * @sa \ref sub_cost_functions
   *
   * @author RevKit
   * @since  1.0
   */
  cost_t costs( const circuit& circ, const cost_function& f );

}

#endif /* COSTS_HPP */
