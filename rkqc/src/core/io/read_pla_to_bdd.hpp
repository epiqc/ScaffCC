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
 * @file read_pla_to_bdd.hpp
 *
 * @brief Reads a BDD from a PLA file
 */

#ifndef READ_PLA_TO_BDD_HPP
#define READ_PLA_TO_BDD_HPP

#include <iostream>
#include <map>
#include <vector>

#include <boost/optional.hpp>

#include <cudd.h>

namespace revkit
{

  /**
   * @brief Contains the result data for read_pla_to_bdd
   *
   * @author RevKit
   * @since  1.3
   */
  class BDDTable
  {
  public:
    /**
     * @brief Default constructor
     *
     * @author RevKit
     * @since  1.3
     */
    BDDTable();

    /**
     * @brief Deconstructor
     *
     * @author RevKit
     * @since  1.3
     */
    ~BDDTable();

    /**
     * @brief Stores the input nodes (with name)
     */
    std::vector<std::pair<std::string, DdNode*> > inputs;

    /**
     * @brief Stores the output nodes (with name)
     */
    std::vector<std::pair<std::string, DdNode*> > outputs;

    /**
     * @brief If the characteristic function is generated (not used so far),
     *        then the real number of inputs is stored here as outputs will
     *        always contain one element in this case.
     */
    boost::optional<unsigned> num_real_outputs;

    /**
     * @brief The internal CUDD manager object
     */
    DdManager* cudd;
  };

  /**
   * @brief Reads a BDD from a PLA file
   *
   * @author RevKit
   * @since  1.3
   */
  bool read_pla_to_bdd( BDDTable& bdd, const std::string& filename );

}

#endif /* READ_PLA_TO_BDD_HPP */
