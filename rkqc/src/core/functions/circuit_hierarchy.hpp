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
 * @file circuit_hierarchy.hpp
 *
 * @brief Returns the hierarchy of a circuits by its modules
 */

#ifndef CIRCUIT_HIERARCHY_HPP
#define CIRCUIT_HIERARCHY_HPP

#include <iostream>

#include <boost/assign/std/vector.hpp>
#include <boost/foreach.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_as_tree.hpp>

/** @cond */
#define foreach BOOST_FOREACH
/** @endcond */

using namespace boost::assign;

namespace revkit
{
  class circuit;

  /** 
   * @brief Hierarchy Graph
   * 
   * Graph concept class for holding a hierarchy graph which
   * is encapsulated inside a tree with hierarchy_tree. For
   * each node it holds the name and the reference of the module
   * inside a tuple stored in the \p vertex_name property.
   *
   * For further information on how to process graphs and
   * trees, please refer to the <a href="http://www.boost.org/doc/libs/1_43_0/libs/graph/doc/index.html" target="_blank">Boost Graph Libary</a>.
   *
   * @author RevKit
   * @since  1.1
   */
  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS, boost::property<boost::vertex_name_t, boost::tuple<std::string, const circuit*> > > hierarchy_graph;


  /** 
   * @brief This class represents a tree based on a Boost.Graph
   *
   * This class uses the boost::graph_as_tree, but offers
   * convinient methods for access the root, children, and
   * parents. 
   *
   * @author RevKit
   * @since  1.1
   */
  template<typename Graph>
  class constructible_tree
    : public boost::graph_as_tree<Graph,
                                  boost::iterator_property_map<typename std::vector<typename boost::graph_traits<Graph>::vertex_descriptor>::iterator, typename boost::property_map<Graph, boost::vertex_index_t>::type>,
                                  typename boost::graph_traits<Graph>::vertex_descriptor>
  {
  public:
    /**
     * @brief Type of super class
     *
     * @author RevKit
     * @since  1.1
     */
    typedef boost::graph_as_tree<Graph,
                                 boost::iterator_property_map<typename std::vector<typename boost::graph_traits<Graph>::vertex_descriptor>::iterator, typename boost::property_map<Graph, boost::vertex_index_t>::type>,
                                 typename boost::graph_traits<Graph>::vertex_descriptor> super;

    /** 
     * @brief Type of the graph
     *
     * @author RevKit
     * @since  1.1
     */
    typedef Graph graph_type;

    /** 
     * @brief Type of a node
     *
     * @author RevKit
     * @since  1.1
     */
    typedef typename boost::graph_traits<Graph>::vertex_descriptor node_type;

    /** 
     * @brief Standard constructor
     * 
     * Initializes default values
     *
     * @author RevKit
     * @since  1.1
     */
    constructible_tree() : super( g, 0 )
    {
    }

    /** 
     * @brief Returns the underlying graph
     * 
     * @author RevKit
     * @since  1.1
     */
    const Graph& graph() const { return g; }

    /** 
     * @brief Returns a mutable reference to the underlying graph
     *
     * @author RevKit
     * @since  1.1
     */
    Graph& graph() { return g; }

    /** 
     * @brief Returns the root of the graph
     * 
     * @return Node
     *
     * @author RevKit
     * @since  1.1
     */
    node_type root() const { return super::_root; }

    /** 
     * @brief Sets the root of the graph 
     * 
     * @param root Node
     *
     * @author RevKit
     * @since  1.1
     */
    void set_root( node_type root ) { super::_root = root; }

    /** 
     * @brief Returns the parent of a node
     * 
     * @param n Node
     * 
     * @return Parent of node \p n
     *
     * @author RevKit
     * @since  1.1
     */
    node_type parent( node_type n ) const
    {
      typename boost::graph_traits<Graph>::in_edge_iterator it, itEnd;
      boost::tie( it, itEnd ) = boost::in_edges( n, g );
      
      if ( it == itEnd ) /* root */
      {
        return super::_root; // return root as parent of root
      }
      else if ( std::distance( it, itEnd ) == 1u )
      {
        return boost::source( *it, g );
      }
      else
      {
        assert( false );
      }
    }

    /** 
     * @brief Returns the children of a node
     * 
     * @param n Node
     * @param children Empty list containing the children of node \p n after the call
     *
     * @author RevKit
     * @since  1.1
     */
    void children( node_type n, std::vector<node_type>& children ) const
    {
      foreach ( const node_type& child, boost::adjacent_vertices( n, g ) )
      {
        children += child;
      }
    }

  private:
    Graph g;
  };

  /** 
   * @brief Hierarchy Tree
   *
   * This tree is used to store the hierarchy of a tree.
   * It is based on the hierarchy_graph
   * 
   * @author RevKit
   * @since  1.1
   */
  typedef constructible_tree<hierarchy_graph> hierarchy_tree;

  /** 
   * @brief Calculates the hierarchy of a circuit by its modules
   *
   * This function traverses all modules starting from a top
   * circuit \p circ recursively and builds a tree and stores it in
   * \p tree.
   * 
   * @param circ Circuit
   * @param tree Hierarchy Tree
   *
   * @author RevKit
   * @since  1.1
   */
  void circuit_hierarchy( const circuit& circ, hierarchy_tree& tree );
}

#endif /* CIRCUIT_HIERARCHY_HPP */
