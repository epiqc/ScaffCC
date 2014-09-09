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

#include <fstream>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <iterator>
#include "epsilonnet.h"
#include "output.h"

#include <boost/range.hpp>
using boost::make_iterator_range;

using namespace std;

/// \brief Header of the file with epsilon net
struct epsilonnetHeader
{
    size_t nodes_count;
};

std::ostream& operator<<(std::ostream& out, const enetNode& node )
{
    out << "enetNode[" << node.ipxx << "," << node.ipQxx << ","
        << node.num_offset << "," << node.compl_offset << "]";
    return out;
}

bool epsilonnet::loadFromFile(const char* filename)
{
    epsilonnetHeader eh;
    ifstream ifs( filename, ios_base::binary );
    if( !ifs )
        return false;
    ifs.read( (char*) &eh,sizeof(eh));
    nodes.clear();
    nodes.resize( eh.nodes_count );
    ifs.read( (char*) &nodes[0], nodes.size() * sizeof(enetNode) );
    numbers.clear();
    numbers.resize( nodes.back().num_offset );
    ifs.read( (char*) &numbers[0], numbers.size() * sizeof(ri) );
    ifs.close();
    denominator_exponent = denominatorExponent2();
    return true;
}

epsilonnet::epsilonnet()
{
    enetNode nd={0,0,0,0};
    nodes.push_back(nd);
}

size_t epsilonnet::nodesCount(const char* filename) const
{
    epsilonnetHeader eh;
    ifstream ifs( filename, ios_base::binary );
    ifs.read( (char*) &eh,sizeof(eh));
    ifs.close();
    return eh.nodes_count;
}

void epsilonnet::saveToFile  (const char* filename) const
{
    if( numbers.size() == 0 )
        return;

    epsilonnetHeader eh;
    eh.nodes_count = nodes.size();
    ofstream ofs( filename, ios_base::binary );
    ofs.write( (const char*) &eh,sizeof(eh));

    assert( nodes.back().ipxx == 0 );
    assert( nodes.back().ipQxx == 0 );
    assert( nodes.back().num_offset == numbers.size() );

    ofs.write( (const char*) &nodes[0], nodes.size() * sizeof(enetNode) );
    ofs.write( (const char*) &numbers[0], numbers.size() * sizeof(ri) );

    ofs.close();
}

/// \brief Comparator based on pointers data
template< class T>
struct eq_ptr
{
    bool operator() ( const T* a , const T* b )
    {
        return *a == *b;
    }
};

/// \brief Modified stub of back_insert_iterator from http://www.cplusplus.com/reference/std/iterator/back_insert_iterator/
template <class Container>
  class back_insert_iterator_ptr :
    public iterator<output_iterator_tag,void,void,void,void>
{
protected:
  Container* container;

public:
  typedef Container container_type;
  explicit back_insert_iterator_ptr (Container& x) : container(&x) {}
  back_insert_iterator_ptr<Container>& operator= (const typename Container::value_type* value)
    { container->push_back(*value); return *this; }
  back_insert_iterator_ptr<Container>& operator* ()
    { return *this; }
  back_insert_iterator_ptr<Container>& operator++ ()
    { return *this; }
  back_insert_iterator_ptr<Container> operator++ (int)
    { return *this; }
};

void epsilonnet::addNode( const pair< ip_type,ip_type>& ip, const nodeRangesPtr &ranges)
{
    nodes.back().ipxx = ip.first;
    nodes.back().ipQxx = ip.second;

    eq_ptr< ri > eq_ri;
    back_insert_iterator_ptr< vector<ri> > biit( numbers );

    int size = numbers.size();
    unique_copy( ranges.nums_begin, ranges.nums_end , biit , eq_ri );
    nodes.back().compl_offset = numbers.size() - size;
    unique_copy( ranges.nums_compl_begin, ranges.nums_compl_end , biit , eq_ri );

    enetNode en = {0,0,numbers.size(),0};
    nodes.push_back(en);
}

void epsilonnet::addNode(epsilonnet::ip_type ipxx, epsilonnet::ip_type ipQxx, const nodeRanges &ranges)
{
    nodes.back().ipxx = ipxx;
    nodes.back().ipQxx = ipQxx;

    auto bi = back_inserter( numbers );
    int size = numbers.size();
    unique_copy( ranges.nums_begin, ranges.nums_end ,bi );
    nodes.back().compl_offset = numbers.size() - size;
    unique_copy( ranges.nums_compl_begin, ranges.nums_compl_end , bi );

    enetNode en = {0,0,numbers.size(),0};
    nodes.push_back(en);
}

void epsilonnet::getNode(size_t node_id, nodeRanges &ranges) const
{
    const enetNode& node = nodes[ node_id ];
    ranges.nums_begin = numbers.begin() + node.num_offset;
    ranges.nums_end = numbers.begin() + complOffset( node_id );
    ranges.nums_compl_begin = ranges.nums_end;
    ranges.nums_compl_end = numbers.begin() + nodes[ node_id + 1 ].num_offset;
}

size_t epsilonnet::complOffset(size_t nodeId) const
{
    const enetNode& node = nodes[ nodeId ];
    return node.num_offset + node.compl_offset;
}

int epsilonnet::denominatorExponent2() const
{
    size_t offset = complOffset(0);
    auto denom = numbers[offset].ipxx() + numbers[0].ipxx();
    int val = ri::gde2( denom );
    // overflow check
    decltype( denom ) d = 1;
    d <<= val;
    assert( d == denom && "Sum of ipxx for number and complementary number is not a power of 2." );
    //denominator_exponent = val;
    return val;
}

int epsilonnet::sde() const
{
    int res = ::sde( 2 * denominatorExponent2(), numbers[0].abs2().gde() );
    if( res == std::numeric_limits<int>::max() ) res = 0;
    return res;
}

double epsilonnet::findExhaustiveApproximation(const epsilonnet::vector2double &vec, epsilonnet::vi &result) const
{
    //denominator_exponent = denominatorExponent2();
    vi current;
    double current_dist = 1.0;
    double best_dist = 1.0;
    vector2double canonical_vec = vec;
    bool conj_1 = false, conj_2 = false;
    int w_pow1 = 0, w_pow2 = 0;
    canonical_vec.first = canonical( vec.first, w_pow1, conj_1 );
    canonical_vec.second = canonical( vec.second, w_pow2, conj_2 );

    for( int i = 0; i < nodes.size(); ++i )
    {
        current_dist = findExhaustiveApproximation( canonical_vec, current, i );
        if( best_dist > current_dist )
        {
            best_dist = current_dist;
            result  = current;
        }
    }

//    cout << result.de << ":" << endl;
//    cout <<  canonical_vec.first << result.d[0].toComplex( result.de ) << endl;
//    cout <<  canonical_vec.second << result.d[1].toComplex( result.de ) << endl;

    if( conj_1 ) result.d[0].conjugate_eq();
    if( conj_2 ) result.d[1].conjugate_eq();
    result.d[0].mul_eq_w( w_pow1 );
    result.d[1].mul_eq_w( w_pow2 );
    result.de = denominator_exponent;

    return best_dist;
}

typedef  epsilonnet::vector2double vd;

/// \brief Inline Euclidean distance computation
inline double dist_squared( const vd& a, const vd& b )
{
    double d1 = a.first.real() - b.first.real();
    double d2 = a.second.real() - b.second.real();
    double d3 = a.first.imag() - b.first.imag();
    double d4 = a.second.imag() - b.second.imag();
    return d1*d1 +d2*d2 +d3*d3 + d4*d4;
}

/// \brief Inline Euclidean distance computation
inline double dist_squared( const vd& a, double fre,  double fim , double sre ,  double sim )
{
    double d1 = a.first.real() - fre;
    double d2 = a.second.real() - sre;
    double d3 = a.first.imag() - fim;
    double d4 = a.second.imag() - sim;
    return d1*d1 +d2*d2 +d3*d3 + d4*d4;
}

/// \todo Remove code duplication by changing one of epsilonnet::findExhaustiveApproximation
double epsilonnet::findExhaustiveApproximation(const epsilonnet::vector2double &vec, epsilonnet::vi &result, int node_id) const
{
    double best = 3.0;
    nodeRanges nr;
    getNode( node_id, nr );
    auto ic = nr.nums_begin;
    auto jc = nr.nums_compl_begin;
    bool tw = true;
    for( auto i = nr.nums_begin; i != nr.nums_end ;++i )
    {
        double iim, ire;
        i->toComplex( denominator_exponent, ire, iim );
        for( auto j = nr.nums_compl_begin; j != nr.nums_compl_end; ++j )
        {
            double jim, jre;
            j->toComplex( denominator_exponent, jre, jim  );
            double d1 = dist_squared( vec, ire, iim, jre, jim );
            double d2 = dist_squared( vec, jre, jim, ire, iim );

            if( d1 < best )
            {
                best = d1;
                ic = i; jc = j;
                tw = false;
            }

            if( d2 < best )
            {
                best = d2;
                ic = i; jc = j;
                tw = true;
            }
        }
    }

    if( tw )
    {
        result.d[0] = *jc;
        result.d[1] = *ic;
        //result.de = denominator_exponent; // -avoid extra operations
    }
    else
    {
        result.d[0] = *ic;
        result.d[1] = *jc;
        //result.de = denominator_exponent; // -avoid extra operations
    }

    return best;
}

double epsilonnet::findExhaustiveApproximation(const epsilonnet::vector2double &vec, epsilonnet::vi &result, int node_id, double &bdist) const
{
    nodeRanges nr;
    getNode( node_id, nr );
    bool found = false;
    auto ic = nr.nums_begin;
    auto jc = nr.nums_compl_begin;
    bool tw = true;
    for( auto i = nr.nums_begin; i != nr.nums_end ;++i )
    {
        double iim, ire;
        i->toComplex( denominator_exponent, ire, iim );
        for( auto j = nr.nums_compl_begin; j != nr.nums_compl_end; ++j )
        {
            double jim, jre;
            j->toComplex( denominator_exponent, jre, jim  );
            double d1 = dist_squared( vec, ire, iim, jre, jim );
            double d2 = dist_squared( vec, jre, jim, ire, iim );

            if( d1 < bdist )
            {
                found = true;
                bdist = d1;
                ic = i; jc = j;
                tw = false;
            }

            if( d2 < bdist )
            {
                found = true;
                bdist = d2;
                ic = i; jc = j;
                tw = true;
            }
        }
    }

    if( found )
    {
        if( tw )
        {
            result.d[0] = *jc;
            result.d[1] = *ic;
            result.de = denominator_exponent;
        }
        else
        {
            result.d[0] = *ic;
            result.d[1] = *jc;
            result.de = denominator_exponent;
        }
    }

    return bdist;
}

//// Code that was used to iterate through epsilon net. Does not required now

//private:
//    /// \brief Executes functor for all complex values in epsilon net
//    template< class T>
//    void for_all_complex( T& functor ) const;
//    /// \brief Executes functor for all complex values in epsilon net within node
//    template< class T>
//    void for_all_complex( T& functor, int node_id ) const;
//    /// \brief Executes functor for all complex values in epsilon net within node with fixed first element
//    template< class T>
//    void for_all_complex( T& functor, nodeRanges& nr, ri& a ) const;
//    /// \brief Executes functor for a, b and elements that are of the form \f$ a\omega^k, a\omega^k \f$
//    template< class T>
//    void for_all_complex( T& functor, std::complex<double>& a, ri& b ) const;

//template< class T>
//void epsilonnet::for_all_complex( T& functor ) const
//{
//    denominator_exponent = denominatorExponent2();
//    for( int i = 0; i < nodes.size() - 1 ; ++i )
//        for_all_complex( functor, i );
//}

//template< class T>
//void epsilonnet::for_all_complex( T& functor, int node_id ) const
//{
//    nodeRanges nr;
//    getNode( node_id, nr );
//    for( auto i : make_iterator_range( nr.nums_begin, nr.nums_end) )
//    {
//        ri a(i);
//        if( i.is_im_eq0() )
//        {
//            for_all_complex( functor, nr, a );
//        }
//        else
//        {
//            for_all_complex( functor, nr, a );
//            a.conjugate_eq();
//            for_all_complex( functor, nr, a );
//        }
//    }
//}

//template< class T>
//void epsilonnet::for_all_complex( T& functor, nodeRanges& nr, ri& a ) const
//{
//    for( int i = 0; i < 8 ; ++i )
//    {
//        a.mul_eq_w();
//        auto first = a.toComplex( denominator_exponent );
//        for( auto j : make_iterator_range(nr.nums_compl_begin, nr.nums_compl_end) )
//        {
//            ri b(j);
//            if( j.is_im_eq0() )
//            {
//                for_all_complex( functor, first, b );
//            }
//            else
//            {
//                for_all_complex( functor, first, b );
//                b.conjugate_eq();
//                for_all_complex( functor, first, b );
//            }
//        }
//    }
//}

//template< class T>
//void epsilonnet::for_all_complex( T& functor, std::complex<double>& first, ri& b ) const
//{
//    int max_ph = 1;
//    if( all_phases ) max_ph = 8;

//    for( int i = 0; i < max_ph ; ++i )
//    {
//        b.mul_eq_w();
//        auto second = b.toComplex( denominator_exponent );
//        functor( first, second );
//        functor( second, first );
//    }
//}
