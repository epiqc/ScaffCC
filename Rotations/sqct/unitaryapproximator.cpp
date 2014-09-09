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

#include "unitaryapproximator.h"
#include "netgenerator.h"
#include <iostream>
#include <algorithm>
#include <ctime>
#include <cassert>
#include <fstream>
#include "exactdecomposer.h"

using namespace std;

/// \brief Order on index nodes based on index_node::abs2 used in indexedUnitaryApproximator
struct indexNodeComparator
{
    bool operator () ( const index_node& a, const index_node& b ) const
    {
        return a.abs2 < b.abs2;
    }
};

/////////////////////////////////////////////////////

unitaryApproximator::unitaryApproximator( int max_layer )
{
    for( int i = 0; i < max_layer; ++i )
    {
        layers.push_back( unique_ptr<epsilonnet>( new epsilonnet ) );
        if( ! layers.back()->loadFromFile( netGenerator::fileName(i).c_str() ) )
        {
            //cout << "layer# " << i << " skipped" << endl;
            layers.pop_back();
        }
        layers.back()->denominatorExponent2(); //set up denominator exponent
    }
}

double unitaryApproximator::approximate(const unitaryApproximator::Ma &m, unitaryApproximator::Me &res)
{
    assert( abs( m[0][0]*m[1][1] - m[0][1]*m[1][0] - 1.0 ) < 1e-9 );
    epsilonnet::vector2double vd( m[0][0], m[1][0] );
    epsilonnet::vi tmp,r;
    double distSquared = 10.0, bestDist = 10.0 ;
    for( const auto& i : layers )
    {
        distSquared = i->findExhaustiveApproximation( vd,tmp );
        if( distSquared < bestDist )
        {
            r = tmp;
            bestDist = distSquared;
        }
    }
    res = vector2<mpz_class>( (ring_int<mpz_class> ) r[0], (ring_int<mpz_class> ) r[1], r.de );
    return sqrt( 2.0 * bestDist);
}

double unitaryApproximator::statistics(const unitaryApproximator::Ma &m, unitaryApproximator::Me &res)
{
    assert("");
    throw new logic_error("!not implemented");
    return 0.;
}

/////////////////////////////////////////////////////

double indexedUnitaryApproximator::approximate(const unitaryApproximator::Ma &m, unitaryApproximator::Me &res)
{
    assert( abs( m[0][0]*m[1][1] - m[0][1]*m[1][0] - 1.0 ) < 1e-9 );
    vec = epsilonnet::vector2double( m[0][0],m[1][0] );
    bestDist = 10.0;
    abs2val = norm ( vec.first ); // absolute value squared of the first component of the column

    epsilonnet::vector2double tmp = vec;
    bool conj_1 = false, conj_2 = false;
    int w_pow1 = 0, w_pow2 = 0;
    vec.first = canonical( tmp.first, w_pow1, conj_1 );
    vec.second = canonical( tmp.second, w_pow2, conj_2 );

    double epsilon0 = 0.001;
    approximate_i( 0, 0 , epsilon0 );

    if( conj_1 ) curr_res.d[0].conjugate_eq();
    if( conj_2 ) curr_res.d[1].conjugate_eq();
    curr_res.d[0].mul_eq_w( w_pow1 );
    curr_res.d[1].mul_eq_w( w_pow2 );

    res.d[0][0] = (ring_int<mpz_class> )curr_res.d[0];
    res.d[1][0] = (ring_int<mpz_class> )curr_res.d[1];
    res.d[0][1] = (ring_int<mpz_class> ) - curr_res.d[1].conjugate();
    res.d[1][1] = (ring_int<mpz_class> ) curr_res.d[0].conjugate();
    res.de = curr_res.de;
    return sqrt( 2.0 * bestDist );
}

typedef ring_int<int>::mpclass mpclass;

double indexedUnitaryApproximator::statistics(const unitaryApproximator::Ma &m, unitaryApproximator::Me &res)
{
    matrix2x2hpr mhpr(m);
    assert( abs( m[0][0]*m[1][1] - m[0][1]*m[1][0] - 1.0 ) < 1e-9 );
    epsilonnet::vector2double vd( m[0][0], m[1][0] );
    epsilonnet::vi tmp,r;
    double distSquared = 10.0, bestDist = 10.0 ;
    //cout << "{" << endl;
    double total_sec = 0;
    exactDecomposer ed;
    for( const auto& i : layers )
    {
        auto t1 = clock();
        distSquared = i->findExhaustiveApproximation( vd,tmp );
        auto t2 = clock();
        float sec = ((float)(t2 - t1)) / CLOCKS_PER_SEC;
        total_sec += sec;
        //cout.setf(ios::fixed,ios::floatfield);
        //cout.precision(16);

        circuit c;
        matrix2x2<mpz_class> m((ring_int<mpz_class>)tmp[0],(ring_int<mpz_class>)-tmp[1].conjugate(),
                               (ring_int<mpz_class>)tmp[1],(ring_int<mpz_class>)tmp[0].conjugate(),
                               tmp.de );
        ed.decompose( m ,c );
        auto counts = gateLibrary::toCliffordT( c.count() );
        int total = counts[ gateLibrary::T ] + counts[ gateLibrary::Td ];

        distSquared = trace_dist(mhpr,(matrix2x2hpr)m);
        if( distSquared < bestDist )
        {
            r = tmp;
            bestDist = distSquared;
        }
        //cout << "{" << total << "," << distSquared << "},"<< endl;
    }
    res = vector2<mpz_class>( (ring_int<mpz_class> ) r[0], (ring_int<mpz_class> ) r[1], r.de );
    //cout << "{" << res.max_sde_abs2() << "," << sqrt(bestDist) <<  "," << total_sec << "}}"<< endl;;
    return bestDist;
}

void indexedUnitaryApproximator::add_nodes_to_index(int layer_id)
{
    const epsilonnet& enet = *layers[layer_id].get();
    int de = enet.denominatorExponent2();
    int sz = enet.nodes.size();
    const auto& nodes = enet.nodes;
    index_nodes.reserve( index_nodes.size() + nodes.size() );
    for( int i = 0; i < sz; ++i )
    {
        ring_int_real<int> rreal(nodes[i].ipxx,nodes[i].ipQxx);
        double re,im;
        rreal.toComplex(2*de,re,im);
        assert( re >= 0.0 && re <= 1.0 );
        index_nodes.push_back( {re,i,(short)layer_id,0} );
        index_nodes.push_back( {1.0 - re,i,(short)layer_id,1} );
    }
}

/// \brief Header of index file
struct inxdex_header
{
    size_t size;
};

void indexedUnitaryApproximator::loadIndex()
{
    inxdex_header eh = {0};
    stringstream filename;
    filename << "/tmp/index-" << layers.size() << ".bin";
    ifstream ifs( filename.str().c_str(), ios_base::binary );
    if( !ifs )
    {
        is_index_ok = false;
        return;
    }
    ifs.read( (char*) &eh,sizeof(eh));
    if( eh.size == 0 )
    {
        is_index_ok = false;
        return;
    }
    index_nodes.clear();
    index_nodes.resize( eh.size );
    ifs.read( (char*) &index_nodes[0], index_nodes.size() * sizeof(index_node) );
    is_index_ok = true;
}

void indexedUnitaryApproximator::approximate_i(size_t end1, size_t start2, double epsilon0)
{
    // In the beginning we make an assumption that there exist approximation
    // within Euclidean distance epsilon0, in this case we can find
    // bounds on absolute value squared of the first entry of approximating vector and
    // reduce amount of nodes that we need to check:

    double upper_bound = min( 1.0, sqrt(abs2val) + epsilon0 );
    double lower_bound = max( 0.0, sqrt(abs2val) - epsilon0 );
    index_node low({lower_bound * lower_bound,0,0,0});
    index_node high({upper_bound * upper_bound,0,0,0});
    indexNodeComparator ic;

    // first we find sub-interval of interval [0,index_nodes.size() )
    // such that all values of index_nodes[i].abs2 of this interval
    // belong to [lower_bound,upper_bound]

    auto lb = std::lower_bound( index_nodes.begin(),index_nodes.end(),low,ic);
    auto ub = std::upper_bound( index_nodes.begin(),index_nodes.end(),high,ic);


    int loff = lb - index_nodes.begin();
    int uoff = ub - index_nodes.begin();
    if( ub == index_nodes.end() )
        uoff = index_nodes.size();

    //Now we are looking for best approximation for nodes in the intersection of
    // On the first iteration end = start amd we effectively search inside [loff, uoff )
    if( end1 == 0 && start2 == 0 )
        end1 = start2 = ( loff + uoff ) / 2;

    // On next iteration we update end1 and start2 to exclude intervals that we already checked
    // [loff,end1) U [start2, uoff )

    for( int i = loff; i < end1 ; ++i )
    {
        index_node& cn = index_nodes[i];
        layers[ cn.layer_id ]->findExhaustiveApproximation(vec,curr_res,cn.node_id,bestDist);
    }

    for( int i = start2; i < uoff; ++i )
    {
        index_node& cn = index_nodes[i];
        layers[ cn.layer_id ]->findExhaustiveApproximation(vec,curr_res,cn.node_id,bestDist);
    }

    auto v1 = curr_res.d[0].toComplex( curr_res.de );
    auto v2 = curr_res.d[1].toComplex( curr_res.de );

    // if we find something closer then epsilon0 we finish algorithm
    if( ( norm(v1-vec.first) + norm(v2-vec.second) ) < epsilon0 * epsilon0 )
        return;
    else
    // otherwise we continue search with bigger epsilon ( we relax initial assumption )
        approximate_i(loff,uoff,epsilon0 + 0.005 );
}

indexedUnitaryApproximator::indexedUnitaryApproximator(int max_layer) :
    unitaryApproximator( max_layer ), is_index_ok(false)
{
    loadIndex();
    // if( is_index_ok )
    //     cout << "index loaded" << endl;
    if( !is_index_ok )
        createIndex();

}

void indexedUnitaryApproximator::createIndex()
{
    for( int i = 0; i < layers.size() ; ++i )
        add_nodes_to_index( i );

    indexNodeComparator ic;
    std::sort( index_nodes.begin(), index_nodes.end(), ic );

    { // write to file
        stringstream filename;
        filename << "/tmp/index-" << layers.size() << ".bin";
        ofstream ofs( filename.str().c_str(), ios_base::binary );
        if( !ofs )
            throw std::exception();
        inxdex_header eh = {index_nodes.size()};
        ofs.write( (const char*) &eh,sizeof(eh));
        ofs.write( (const char*) &index_nodes[0], index_nodes.size() * sizeof(index_node) );
        ofs.close();
    }
}
