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

#include "netgenerator.h"
#include "numbersgen.h"
#include "epsilonnet.h"

#include <memory>
#include <deque>
#include <vector>
#include <algorithm>
#include <sstream>
#include <functional>
#include <cassert>

using namespace std;

void netGenerator::generateInitial()
{
    typedef ring_int<int> ri;

    unique_ptr< numbersGenerator<10> > png( new numbersGenerator<10>);
    auto& ng = *png.get();
    ng.generate_all_numbers();

    epsilonnet enets[ ng.max_sde + 1 ];

    for( int i = 0; i < ng.ip_range; ++i )
    for( int j = 0; j < ng.max_val2 + 1; ++j )
    {
        const auto& vec = ng.numbers(i,j);
        const auto& vec_compl = ng.numbers( ng.max_val2 - i, -j);

        if( (!vec.empty() ) && (!vec_compl.empty() ))
        {
            int s = std::min ( ng.getSde(i,j), ng.getSde(ng.max_val2 - i, -j) );
            int de = ng.max_sde / 2  - (s + 1 ) / 2;

            vector<ri> v;
            vector<ri> vc;

            // normalize denominator and pick canonical representatives
            for( auto ii : vec )
            {
                v.push_back( ii.canonical() );
                v.back().div_eq_sqrt2( de );
            }

            for( auto ii : vec_compl )
            {
                vc.push_back( ii.canonical() );
                vc.back().div_eq_sqrt2( de );
            }
            // sort to leave only unique elements
            sort( v.begin(), v.end() );
            sort( vc.begin(), vc.end() );

            nodeRanges nr = {v.begin(),v.end(),vc.begin(), vc.end() };
            assert( ( (i >> de) << de ) == i );
            assert( ( (j >> de) << de ) == j );
            // in the case when ipQxx = 0 we only add numbers such that ipxx <= ng.max_val2 / 2
            // the number with ipxx >= ng.max_val2 / 2 will be in complementary part automatically
            if( ( j != 0 ) || ( (j == 0) && (i <= ng.max_val2 / 2) ) )
                enets[s].addNode(i >> de ,j >> de ,nr);
        }
    }

    for( int i = 0; i <= ng.max_sde; ++i )
    {
        enets[i].saveToFile( fileName(i).c_str() );
    }
}

string netGenerator::fileName(int sde)
{
    stringstream s;
    char const *folder = getenv("TMPDIR");
    if (folder == 0)
        folder = "/tmp";
    s << folder << "/epsilon-net." << sde << ".bin";
    return s.str();
}


///////////////// internal implementation of generator ///////////////////////////////////

typedef  netGenerator::ri ri;

/// \brief Node of generated data
struct gdata
{
    /// \brief ip.first is P(x) of num and ip.second is Q(x) of num  \see ring_int::ipxx() for defintions of P(x) and Q(x)
    pair<ri::pr_type,ri::pr_type> ip;
    /// \brief Found number
    ri num;
    /// \brief Found complementary number \see enetNode::compl_offset
    ri num_compl;
};

/// \brief Order on pointers to gdata based on values of P(x) and Q(x) \see ring_int::ipxx() for defintions of P(x) and Q(x)
struct gdata_comparator
{
    bool operator () ( const gdata* a, const gdata* b) const
    {
        return a->ip < b->ip;
    }
};

/// \brief Order on pointers based on data that they point to
template< class T >
struct ptr_comp : public binary_function< const T*, const T*, bool >
{
    const bool operator() ( const T* a, const T* b)
    {
        return *a < *b;
    }
};

/// \brief Internal structure for epsilon net generation
struct generation_data
{
    /// \brief Sets sde end denominator exponent used during generation
    generation_data( int sde, int de2 )
    {
        isGde1 = (sde % 2 == 1);
        pow2n = 1 << (isGde1 ? de2 : de2 + 1 );
        pow2nm1 = pow2n >> 1;
    }

    /// \brief Found values of P(x) and Q(x)
    deque< gdata > ips;
    /// \brief Sum of P(x) and P(y) for two complementary numbers \see epsilonnet::compl_offset
    ri::pr_type pow2n;
    /// \brief If P(x) is less than pow2nm1 we store (x,y), and (y,x) otherwise
    ri::pr_type pow2nm1;
    /// \brief Pointers to computed numbers
    vector< const gdata* > order;
    /// \brief When sde is even then \f$gde(|\cdot|^2)\f$ 0 and 1 otherwise. See properties of gde and sde in
    /// http://arxiv.org/abs/1206.5236
    bool isGde1;

    /// \brief Adds all numbers to epsilon net that we can get from unit column based on (omega_k_y,x)
    void add_for_all_k ( ri& omega_k_y, const ri& x )
    {
        for( int k = 0; k < 8; ++k )
        {
            omega_k_y.mul_eq_w(); //goes to next element in eq. class
            ri z = x + omega_k_y;
            ri w = x - omega_k_y;
            auto ipxx  = z.ipxx();
            auto ipQxx = z.ipQxx();

            if( isGde1 )
            {
                // in this case \f$gde(|\cdot|^2)=2\f$ and we need further reduction
                if( ri::isGde2(ipxx,ipQxx) )
                {
                    ipxx >>= 1;
                    ipQxx >>= 1;
                    z.div_eq_sqrt2();
                    w.div_eq_sqrt2();
                    push_back(ipxx,ipQxx,z,w);
                }
            }
            else
            {
                if( ri::isGde1(ipxx,ipQxx) )
                    push_back(ipxx,ipQxx,z,w);
            }

        }
    }

    /// \brief Adds number equivalence class to epsilon net. By calling ring_int::canonical
    /// we mod out multiplication by power of \f$ \omega \f$ and conjugation.
    /// By comparison of ipxx with pow2nm1 we mod out permutation of column entries
    void push_back( ri::pr_type ipxx, ri::pr_type ipQxx, const ri& a, const ri& ac )
    {
        if( ipQxx > 0 )
            ips.push_back( { {ipxx, ipQxx}, a.canonical(),ac.canonical()} );
        else if( ipQxx == 0 )
        {
            if( ipxx < pow2nm1 )
                ips.push_back( {{ipxx, 0},a.canonical(),ac.canonical()} );
            else
                ips.push_back( {{pow2n - ipxx,0},ac.canonical(),a.canonical()} );

        } else if ( ipQxx < 0 )
            ips.push_back( {{pow2n - ipxx,-ipQxx},ac.canonical(),a.canonical()} );
    }

    /// \brief Builds epsilon net from computed data
    epsilonnet* build_net()
    {

        epsilonnet* enet = new epsilonnet;
        order.resize( ips.size() );
        int sz = order.size();
        for( int i = 0; i < sz; ++i )
            order[i] = & ips[i];

        gdata_comparator gcomp;
        /// \todo This sort can be avoided with a bit more clever algorithm
        sort( order.begin(), order.end(), gcomp );

        int j = 0;
        int j_end = 0;

        for( int i = 0; i < sz - 1; ++i )
        {
            if( gcomp( order[i], order[i+1] ) )
            {
                j_end = i + 1;
                add_to_net(j,j_end,enet);
                j = j_end;
            }
        }
        j_end = sz;
        add_to_net(j,j_end,enet);
        return enet;
    }

    /// \brief Adds range from \b order to epsilon net
    void add_to_net( int j, int j_end ,epsilonnet* enet )
    {
        vector< const ri* > nums,nums_compl;

        for( int i = j ; i < j_end; ++i )
        {
            nums.push_back( &(order[i]->num) );
            nums_compl.push_back( &( order[i]->num_compl ) );
        }

        ptr_comp<ri> ri_comp;
        sort( nums.begin(), nums.end(), ri_comp );
        sort( nums_compl.begin(), nums_compl.end(), ri_comp );

        nodeRangesPtr nr = { nums.begin(), nums.end(),
                             nums_compl.begin(), nums_compl.end() };

        enet->addNode( order[j]->ip, nr);
    }
};

epsilonnet* netGenerator::generate(const epsilonnet& enet)
{
    generation_data gdata( enet.sde(), enet.denominatorExponent2() );

    for( int i = 0; i < enet.nodes.size() - 1; ++i )
    {
        const enetNode& node = enet.nodes[i];

        nodeRanges r;
        enet.getNode( i, r );

        for( auto i1 = r.nums_begin; i1 != r.nums_end; ++i1 )
        {
            const ri& x = *i1;
            ri x_c = x.conjugate();

            for( auto i2 = r.nums_compl_begin; i2 != r.nums_compl_end; ++i2 )
            {
                // addes all vectors that can be generated from equivalence class
                ri omega_k_y(*i2);
                gdata.add_for_all_k( omega_k_y, x );

                ri omega_k_y_c( i2->conjugate() );
                gdata.add_for_all_k( omega_k_y_c, x );

            }
        }
    }

    return gdata.build_net();
}
