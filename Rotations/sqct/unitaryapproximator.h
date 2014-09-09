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

#ifndef UNITARYAPPROXIMATOR_H
#define UNITARYAPPROXIMATOR_H

#include "matrix2x2.h"
#include "epsilonnet.h"
#include <memory>
#include <vector>

/// \brief Perfors approximation of machine precision unitaries by exact
/// unitaries over the ring  \f$ \mathbb{Z}[\frac{1}{\sqrt{2}},i]\f$
/// \note Class is not thread safe now. Possible solution it to write appropriate copy constructor
/// that will share epsilon nets.
class unitaryApproximator
{
public:
    /// \brief Type of input for approximation - machine precision unitary
    typedef matrix2x2cd Ma;
    /// \brief Type of result of approximation - exact unitary
    typedef matrix2x2<mpz_class> Me;
    /// \brief Loads epsilon nets from files on hard drive
    /// \param max_layer Non inclusive upper bound of \f$ sde(|\cdot|^2) \f$ that will be used for approximation
    unitaryApproximator( int max_layer = 31 );
    /// \brief Performs approximation of the special unitary m and writes result into res
    virtual double approximate( const Ma& m, Me& res );
    /// \brief Outputs statistic about time and approximation quality for each layer
    /// \note Not implemented for this class
    virtual double statistics( const Ma& m, Me& res );
protected:
    /// \brief Vector of pointers to all loaded epsilon nets
    std::vector< std::unique_ptr< epsilonnet > > layers;
};

/// \brief Node of index of columns used by indexedUnitaryApproximator
struct index_node
{
    double abs2;///< Largest amongst absolute values squared of the column entries
    int node_id;///< Number of the node
    short layer_id;///< \f$ sde(|\cdot|^2) \f$ of the entry
    short swapped;///< True if the largest absolute value cooresponds to the second column entry, false othewise
};

/// \brief Perfors approximation of machine precision unitaries by exact
/// unitaries over the ring  \f$ \mathbb{Z}[\frac{1}{\sqrt{2}},i]\f$.
/// Uses index based on absolute values of columns entries to speed up search.
/// \note Class is not thread safe now. Possible solution it to write appropriate copy constructor
/// that will share epsilon nets.
class indexedUnitaryApproximator : public unitaryApproximator
{
public:
    /// \brief Loads epsilon nets from files on hard drive.
    /// If there is no index exists creates one automatically.
    /// \param max_layer Non inclusive upper bound of \f$ sde(|\cdot|^2) \f$ that will be used for approximation
    indexedUnitaryApproximator( int max_layer = 31 );
    /// \brief Computes absolute values squared of column entries and stores them in sorted array
    void createIndex();
    /// \brief Performs approximation of the special unitary m and writes result into res
    double approximate( const Ma& m, Me& res );
    /// \brief Outputs statistics about time, approximation quality and number of T gates in resulting circuits
    double statistics( const Ma& m, Me& res );
private:
    /// \brief Adds all nodes from given layer to index
    void add_nodes_to_index( int layer_id );
    /// \brief Loads index from file
    void loadIndex();
    /// \brief Assumes that there exist approximation within distance epsilon0
    /// and check nodes from index_nodes with index in \f$ [0,end1) \cup [start2, index size ) \f$.
    /// If it fails to find node within epsilon0 it relaxes intial assumption and search further.
    void approximate_i( size_t end1, size_t start2, double epsilon0 );
    /// \brief True if index loaded successfully
    bool is_index_ok;
    epsilonnet::vector2double vec; ///< First column of the special unitary that we approximating
    double bestDist;///< Best distance to approximation that was laready found
    double abs2val;///< Absolute value squared of the first component of vec
    epsilonnet::vi curr_res;///< Current best approximation found
    /// \brief Index nodes sorted by index_node::abs2
    std::vector<index_node> index_nodes;
};

#endif // UNITARYAPPROXIMATOR_H
