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
 * @file target_tags.hpp
 *
 * @brief Predefined target type tags for common gate types
 *
 * @sa \ref sub_target_tags
 */

#ifndef TARGET_TAGS_HPP
#define TARGET_TAGS_HPP

#include <core/circuit.hpp>
#include <core/gate.hpp>

#include <boost/any.hpp>

namespace revkit
{

  /**
   * @brief Target Tag for Toffoli gates.
   *
   * @sa \ref sub_target_tags
   *
   * @author RevKit
   * @since  1.0
   */
  struct toffoli_tag {};


  /**
  * @brief Target Tag for CNOT gates.
  *
  * @sa \ref sub_target_tags
  *
  * @author Adam Holmes
  * @since 1.3
  */
  struct cnot_tag {};


  /**
  * @brief Target Tag for CNOT gates.
  *
  * @sa \ref sub_target_tags
  *
  * @author Adam Holmes
  * @since 1.3
  */
  struct not_tag {};




  /**
   * @brief Target Tag for Fredkin gates.
   *
   * @sa \ref sub_target_tags
   *
   * @author RevKit
   * @since  1.0
   */
  struct fredkin_tag {};

  /**
   * @brief Target Tag for Peres gates.
   *
   * @sa \ref sub_target_tags
   *
   * @author RevKit
   * @since  1.0
   */
  struct peres_tag {
    /**
     * @brief Order of Target Lines
     *
     * If \b true, the order of target lines is interchanged,
     * i.e. the first target has an higher line index than the
     * second one.
     *
     * This needs to be done since target lines are stored
     * in a set in the gate container, such that the order
     * cannot be determined afterwards.
     *
     * @author RevKit
     * @since  1.3
     */
    bool swap_targets;
  };

  /**
   * @brief Target Tag for V gates.
   *
   * @sa \ref sub_target_tags
   *
   * @author RevKit
   * @since  1.0
   */
  struct v_tag {};

  /**
   * @brief Target Tag for V+ gates.
   *
   * @sa \ref sub_target_tags
   *
   * @author RevKit
   * @since  1.0
   */
  struct vplus_tag {};

  /**
   * @brief Target Tag for Modules
   *
   * @sa \ref sub_target_tags
   *
   * @author RevKit
   * @since  1.1
   */
  struct module_tag
  {
    /**
     * @brief Name of the module
     *
     * @author RevKit
     * @since  1.1
     */
    std::string name;

    /**
     * @brief Reference to the circuit
     *
     * Usually the circuit is inside of the
     * circuit modules list.
     *
     * @author RevKit
     * @since  1.1
     */
    boost::shared_ptr<circuit> reference;

    /**
     * @brief Sort order of the target tags
     *
     * This list stores the values from
     * 0 to \em n - 1, where \em n is the
     * number of target lines of this module
     * gate. It maps the target gates to the
     * corresponding line of the module, since
     * the target lines in a gate are stored
     * in an ordered set.
     *
     * @return
     *
     * @author RevKit
     * @since  1.1
     */
    std::vector<unsigned> target_sort_order;
  };

  /**
   * @brief Compares type of a boost::any variable
   *
   * This method is called by is_\em gate functions
   * like is_toffoli().
   *
   * @param operand A variable of type boost::any
   * @return true, if \p operand is of type \p T.
   *
   * @author RevKit
   * @since  1.0
   */
  template<typename T>
  bool is_type( const boost::any& operand )
  {
    return operand.type() == typeid( T );
  }

  /**
   * @brief Checks if two gates have the same type
   *
   * Use this function, since == does not work on gate::type
   * to compare to gates by its type.
   *
   * @param g1 First gate
   * @param g2 Second gate
   * @return true, if they have the same target tag, otherwise false
   *
   * @author RevKit
   * @since  1.0
   */
  bool same_type( const gate& g1, const gate& g2 );

  /**
   * @brief Returns whether a gate is a Toffoli gate
   *
   * @param g Gate
   * @return true, if \p g is a Toffoli gate
   *
   * @author RevKit
   * @since  1.0
   */
  bool is_toffoli( const gate& g );

 /**
   * @brief Returns whether a gate is a CNot gate
   *
   * @param g Gate
   * @return true, if \p g is a CNot gate
   *
   * @author Adam Holmes
   * @since  1.3
   */
  bool is_cnot( const gate& g );

 /**
   * @brief Returns whether a gate is a Not gate
   *
   * @param g Gate
   * @return true, if \p g is a Not gate
   *
   * @author Adam Holmes
   * @since  1.3
   */
  bool is_not( const gate& g );





  /**
   * @brief Returns whether a gate is a Fredkin gate
   *
   * @param g Gate
   * @return true, if \p g is a Fredkin gate
   *
   * @author RevKit
   * @since  1.0
   */
  bool is_fredkin( const gate& g );

  /**
   * @brief Returns whether a gate is a Peres gate
   *
   * @param g Gate
   * @return true, if \p g is a Peres gate
   *
   * @author RevKit
   * @since  1.0
   */
  bool is_peres( const gate& g );

  /**
   * @brief Returns whether a gate is a V gate
   *
   * @param g Gate
   * @return true, if \p g is a V gate
   *
   * @author RevKit
   * @since  1.0
   */
  bool is_v( const gate& g );

  /**
   * @brief Returns whether a gate is a V+ gate
   *
   * @param g Gate
   * @return true, if \p g is a V+ gate
   *
   * @author RevKit
   * @since  1.0
   */
  bool is_vplus( const gate& g );

  /**
   * @brief Returns whether a gate is a module
   *
   * @param g Gate
   * @return true, if \p g is a module
   *
   * @author RevKit
   * @since  1.1
   */
  bool is_module( const gate& g );

}

#endif /* TARGET_TAGS_HPP */
