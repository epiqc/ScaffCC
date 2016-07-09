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
 * @file extend_truth_table.hpp
 *
 * @brief Removes the Don't Care Values of a binary truth table
 */

#ifndef EXTEND_TRUTH_TABLE_HPP
#define EXTEND_TRUTH_TABLE_HPP

#include <core/truth_table.hpp>

namespace revkit
{

  /**
   * @brief Removes the Don't Care Values of a binary truth table
   *
   * This methods fills the incomplete cubes of a truth table.
   *
   * @param spec Truth table
   *
   * @author RevKit
   * @since  1.0
   */
  void extend_truth_table( binary_truth_table& spec );

}

#endif /* EXTEND_TRUTH_TABLE_HPP */
