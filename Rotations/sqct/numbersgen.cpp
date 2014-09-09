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

#include "numbersgen.h"
#include <cassert>

static const int sde_not_computed = -1;

template< int de >
void numbersGenerator<de>::init()
{
    int& first = sdes[0][0];
    int& last = sdes[ip_range - 1][ipQ_range - 1];
    std::fill(&first ,&last + 1,sde_not_computed);
}

template< int de >
bool numbersGenerator<de>::isInRange(int ipxx, int ipQxx) const
{
    return ( ipxx >= 0 ) && ( ipxx < ip_range ) &&
           ( ( ipQxx + ipQ_offset ) >= 0 ) &&
           ( ( ipQxx + ipQ_offset ) < ipQ_range );
}

template< int de >
void numbersGenerator<de>::add_number( int a, int b , int c, int d, int ipxx )
{
    int m1 = ( a == 0 ? 1 : 2 );
    int m2 = ( b == 0 ? 1 : 2 );
    int m3 = ( c == 0 ? 1 : 2 );
    int m4 = ( d == 0 ? 1 : 2 );

    static const int s[2] = {1,-1};

    for ( int i1 = 0; i1 < m1 ; ++i1 )
        for ( int i2 = 0; i2 < m2 ; ++i2 )
            for ( int i3 = 0; i3 < m3 ; ++i3 )
                for ( int i4 = 0; i4 < m4 ; ++i4 )
                {
                    ri x( a * s[i1], b * s[i2], c * s[i3], d * s[i4]);
                    int ipQxx = x.ipQxx();

                    static const double SQRT2 =
                            1.4142135623730950488016887242096980785696718753769;

                    ri abs2x( ipxx, ipQxx, 0, -ipQxx );
                    auto res = abs2x.toComplex( 2 * denom_exp );

                    if( abs( res ) <= 1.0000001 )
                    {
                        if( sdes[ ipxx ][ ipQxx + ipQ_offset ] == sde_not_computed )
                            sdes[ ipxx ][ ipQxx + ipQ_offset ]
                                    = sde( 2 * denom_exp,  abs2x.gde() );
                        vals[ ipxx ][ ipQxx + ipQ_offset ].push_back( x );
                        m_total_numbers++;
                    }
                }
}

template< int de >
void numbersGenerator<de>::generate_all_numbers()
{
    for( int i1 = 0; i1 <= max_val; ++i1 )
    {
        int ipxx1 = i1 * i1;
        for( int i2 = 0; i2 <= max_val; ++i2 )
        {
            int ipxx2 = ipxx1 + i2 * i2;
            if( ipxx2 > max_val2 )
                break;
            for( int i3 = 0; i3 <= max_val; ++i3 )
            {
                int ipxx3 = ipxx2 + i3 * i3;
                if( ipxx3 > max_val2 )
                    break;
                for( int i4 = 0; i4 <= max_val; ++i4 )
                {
                    int ipxx4 = ipxx3 + i4 * i4;
                    if( ipxx4 > max_val2 )
                        break;
                    add_number( i1, i2, i3, i4, ipxx4 );
                }
            }
        }
    }
}

template< int de >
numbersGenerator<de>::numbersGenerator() :
    m_total_numbers(0)
{
    assert( denom_exp % 2 == 0 );
    init();
}

template< int de >
int numbersGenerator<de>::getSde(int ipxx, int ipQxx) const
{
    assert( isInRange(ipxx,ipQxx) );
    return sdes[ipxx][ipQ_offset + ipQxx];
}

template< int de >
const typename numbersGenerator<de>::riArray& numbersGenerator<de>::numbers(int ipxx, int ipQxx) const
{
    assert( isInRange(ipxx,ipQxx) );
    return vals[ipxx][ipQ_offset + ipQxx];
}

template< int de >
const typename numbersGenerator<de>::riArray& numbersGenerator<de>::numbersWithPair(int ipxx, int ipQxx) const
{
    static const riArray emp;
    const auto& res = numbers( max_val2 - ipxx,-ipQxx );
    if( ! (res.empty() ) )
        return vals[ipxx][ipQxx];
    else
        return emp;
}

//////// template compilation requests

template class numbersGenerator<2>;
template class numbersGenerator<4>;
template class numbersGenerator<6>;
template class numbersGenerator<8>;
template class numbersGenerator<10>;
template class numbersGenerator<12>;

//////////////////////////////////////

template< int de >
void columnsCounter<de>::count_all_columns()
{
    sde_stat.resize(bs::max_sde,0);
    for( int ipxx = 0; ipxx < bs::max_val2 + 1; ++ipxx )
    {
        for( int j = 0; j < bs::ipQ_range; ++j )
        {
            int ipyy = bs::max_val2 - ipxx;
            int ipQxx = j - bs::ipQ_offset;
            int ipQyy = -ipQxx;
            int k = bs::ipQ_offset + ipQyy;

            int sde = bs::sdes[ipxx][j];
            int sde2 = bs::sdes[ipyy][k];

            auto ii_end = bs::vals[ipxx][j].end();
            for( auto ii = bs::vals[ipxx][j].begin(); ii != ii_end; ++ii )
            {
                auto jj_end = bs::vals[ipyy][k].end();
                for( auto jj = bs::vals[ipyy][k].begin(); jj != jj_end; ++jj )
                {
                    int idx =std::min(sde2,sde);
                    sde_stat[idx] ++;
                }
            }
        }
    }
}

//////// template compilation requests
template class columnsCounter<2>;
template class columnsCounter<4>;
template class columnsCounter<6>;
template class columnsCounter<8>;
template class columnsCounter<10>;
template class columnsCounter<12>;
