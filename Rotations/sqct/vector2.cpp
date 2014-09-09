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

#include "vector2.h"
#include "resring.h"

#include <limits>
#include <algorithm>


template < class TInt >
void vector2<TInt>::div_eq_sqrt2_exp(int a)
{
    d[0].div_eq_sqrt2(a);
    d[1].div_eq_sqrt2(a);
}

template < class TInt >
void vector2<TInt>::set( const ring_int<TInt> &z, const ring_int<TInt> &w, int denom_exp )
{
    d[0] =z;
    d[1]= w;
    de = denom_exp;
}

template < class TInt >
const ring_int<TInt>& vector2<TInt>::operator[]( int a ) const
{
    return d[a];
}

template < class TInt >
ring_int<TInt>& vector2<TInt>::operator[]( int a )
{
    return d[a];
}

template < class TInt >
vector2<TInt>::vector2( const ring_int<TInt> &z, const ring_int<TInt> &w, int denom_exp )
{
    set(z,w,denom_exp);
}

template < class TInt >
vector2<TInt>::vector2()
{
    set( ring_int<TInt>(),ring_int<TInt>(),0 );
}

template < class TInt >
int vector2<TInt>::min_gde()
{
    int min_gde = std::numeric_limits<int>::max();

    for( int i = 0; i < 2; ++i )
            min_gde = std::min( d[i].gde(), min_gde );

    return min_gde;
}

template < class TInt >
int vector2<TInt>::reduce()
{
    vector2<TInt>& vec = *this;
    int gde = min_gde();
    if( gde != std::numeric_limits<int>::max() )
    {
        int red_power = std::min ( gde, de );
        vec.div_eq_sqrt2_exp(red_power);
        de -= red_power;
    }
    return de;
}

template < class TInt >
bool vector2<TInt>::operator <( const vector2<TInt>& B ) const
{
    if ( de > B.de )
        return false;
    else if( de < B.de )
        return true;

    for( int i = 0; i < 2; ++i )
    {
        int le = d[i].le(B.d[i]); // +1 -- less, 0 -- equal, -1 greater
        if( le < 0 )
            return false;
        else if( le > 0 )
            return true;
    }
    return false;
}

//////// template compilation requests

template class vector2<int>;
template class vector2<long int>;
template class vector2<mpz_class>;
template class vector2<resring<8>>;
