//     Copyright (c) 2012 Vadym Kliuchnikov sqct(dot)software(at)gmail(dot)com, Dmitri Maslov, Michele Mosca
//
//     This file is part of SQCT.
// 
//     SQCT is free software: you can redistribute it and/or modify
//     it under the terms of the GNU Lesser General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
// 
//     SQCT is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU Lesser General Public License for more details.
// 
//     You should have received a copy of the GNU Lesser General Public License
//     along with SQCT.  If not, see <http://www.gnu.org/licenses/>.
// 

#ifndef EPSILONNET_H
#define EPSILONNET_H

#include "rint.h"
#include "vector2.h"
#include <vector>

/// \brief Node of epsilon net
struct enetNode
{
    /// \brief Type that is used for P(x) and Q(x) \see ring_int::ipxx() and ring_int::ipQxx()
    typedef ring_int<int>::pr_type ip_type;
    /// \brief Value of P(x)
    ip_type  ipxx;
    /// \brief Value of Q(x)
    ip_type  ipQxx;
    /// \brief Index where numbers with given P(x),Q(x) starts in epsilonnet::numbers
    size_t   num_offset;
    /// \brief Index where complementary numbers starts in epsilonnet::numbers.
    /// Two numbers x,y are complementary if P(x) + P(y) is a power of 2 and Q(x) = -Q(y)
    int      compl_offset;
};

/// \brief Represents range of nodes with fixed P(x), Q(x)
struct nodeRanges
{
    /// \brief Type of the ring elements
    typedef ring_int<int> ri;
    /// \brief Type of ranges iterator
    typedef std::vector<ri>::const_iterator cit;

    /// \brief First element of numbers range
    cit nums_begin;
    /// \brief End of numbers range, the interval is [nums_begin,nums_end)
    cit nums_end;
    /// \brief First element of complementary numbers range \see enetNode::compl_offset
    cit nums_compl_begin;
    /// \brief End of complementary numbers range, the interval is [nums_compl_begin,nums_compl_end)
    cit nums_compl_end;
};

/// \brief Represents range of nodes with fixed P(x), Q(x)
struct nodeRangesPtr
{
    /// \brief Type of the ring elements
    typedef ring_int<int> ri;
    /// \brief Type of ranges iterator
    typedef std::vector<const ri*>::const_iterator cit;

    /// \brief First element of numbers range
    cit nums_begin;
    /// \brief End of numbers range, the interval is [nums_begin,nums_end)
    cit nums_end;
    /// \brief First element of complementary numbers range \see enetNode::compl_offset
    cit nums_compl_begin;
    /// \brief End of complementary numbers range, the interval is [nums_compl_begin,nums_compl_end)
    cit nums_compl_end;
};



/// \brief Serializable epsilon net made from the ring elements with a fixed power of \f$ \sqrt{2} \f$ in the denominator
class epsilonnet
{
public:
    /// \brief Type of the ring elements
    typedef ring_int<int> ri;
    /// \brief Type for unit vectors over the ring
    typedef vector2<int> vi;
    /// \brief Type that is used for P(x) and Q(x) \see ring_int::ipxx() and ring_int::ipQxx()
    typedef ri::pr_type ip_type;
    /// \brief Simple complex 2 dimensional vector
    typedef std::pair< std::complex< double> , std::complex< double> > vector2double;

    /// \brief Creates empty epsilon net
    epsilonnet();

    /// \brief Number of nodes in file
    size_t nodesCount (const char* filename) const;
    /// \brief Loads epsilon net from file
    bool loadFromFile(const char* filename);
    /// \brief Saves epsilon net to file
    void saveToFile  (const char* filename) const;

    /// \brief Adds vectors with P(x) = ip.first and Q(x) = ip.second,
    /// \see ring_int::ipxx() and ring_int::ipQxx()
    /// \param ranges Defines location of unit vectors
    void   addNode( const std::pair<ip_type,ip_type>& ip, const nodeRangesPtr& ranges );

    /// \brief Adds unit vectors with P(x) = ipxx and Q(x) = ipQxx
    /// \see ring_int::ipxx() and ring_int::ipQxx()
    /// \param ranges Defines location of unit vectors
    void   addNode( ip_type ipxx, ip_type ipQxx, const nodeRanges& ranges );

    /// \brief Provides access to node by id
    void   getNode( size_t node_id, nodeRanges& ranges ) const;
    /// \brief Finds offset of complementary part for given node
    size_t complOffset( size_t nodeId ) const;
    /// \brief Returns de of a base\f$ 2 \f$ for given \f$\varepsilon-\f$ net. Assumes that it is the same for all elements.
    int    denominatorExponent2() const;
    /// \brief Returns sde of a base\f$ \sqrt{2} \f$ for given \f$\varepsilon-\f$ net. Assumes that it is the same for all elements.
    int    sde() const;
    /// \brief Exhaustively finds the best approximating vector withing given epsilon net.
    /// \note If you create epsilon net on the fly ( not loading it from file ), you need to initialize denominator_exponent before using this method.
    /// \returns Euclidean distance squared to the best approximating vector
    double findExhaustiveApproximation( const vector2double& vec, vi& result ) const;

    /// \brief Vector of epsilon node
    std::vector<enetNode>  nodes;
    /// \brief Vector of numbers that appears in epsilon net
    std::vector<ri>        numbers;
    /// \brief Denominator exponent of epsilon net elements
    int                    denominator_exponent;

    /// \brief Finds exhaustive approximation within fixed node
    double findExhaustiveApproximation( const vector2double& vec, vi& result, int node_id ) const;
    /// \brief Finds exhaustive approximation within fixed node only if distance to the best possible vector is less than ndist.
    /// Do nothing otherwise. Useful to reduce amount of copy operations
    double findExhaustiveApproximation( const vector2double& vec, vi& result, int node_id, double& bdist ) const;

};


#endif // EPSILONNET_H
