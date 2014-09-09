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

#include "resring.h"

template< int TMod>
resring<TMod>::resring() :
    v ( 0 )
{}

template< int TMod>
resring<TMod>::resring(resring<TMod>::int_type val)
{
    set(val);
}


template< int TMod>
resring<TMod>::resring(const mpz_class &val)
{
    // note: works only for TMod = 8
    int b0 = mpz_tstbit( val.get_mpz_t() , 0 );
    int b1 = mpz_tstbit( val.get_mpz_t() , 1 );
    int b2 = mpz_tstbit( val.get_mpz_t() , 2 );
    v = 0;
    v |= b2;
    v <<= 1;
    v |= b1;
    v <<= 1;
    v |= b0;
}

template< int TMod>
resring<TMod>& resring<TMod>::operator +=(const resring& y)
{
    set( v + y.v );
    return *this;
}

template< int TMod>
resring<TMod>& resring<TMod>::operator -=(const resring& y)
{
    set( v - y.v );
    return *this;
}

template< int TMod>
bool resring<TMod>::operator <(const resring<TMod> &y) const
{
    return v < y.v;
}

template< int TMod>
bool resring<TMod>::operator >(const resring &y) const
{
    return v > y.v;
}

template< int TMod>
resring<TMod> resring<TMod>::operator +(const resring<TMod> &y) const
{
    return resring<TMod>( y.v + v );
}

template< int TMod>
resring<TMod> resring<TMod>::operator -(const resring &y) const
{
    return resring<TMod>( v - y.v );
}

template< int TMod>
resring<TMod> resring<TMod>::operator -() const
{
    return resring<TMod>( -v );
}

template< int TMod>
resring<TMod> resring<TMod>::operator *(const resring<TMod> &y) const
{
    return resring<TMod>( y.v * v );
}

template< int TMod>
resring<TMod> resring<TMod>::operator /(const resring<TMod> &e) const
{
    return resring<TMod>( v / e.v );
}

template< int TMod>
resring<TMod> &resring<TMod>::operator /=(const resring<TMod> &e)
{
    set( v /= e.v );
}

template< int TMod>
resring<TMod> &resring<TMod>::operator <<=(int e)
{
    set( v << e );
}

template< int TMod>
resring<TMod> resring<TMod>::operator >> (int e ) const
{
    return resring( v >> e );
}

template< int TMod>
resring<TMod> &resring<TMod>::operator >>=(int e)
{
    v >>= e;
}
template< int TMod>
typename resring<TMod>::int_type resring<TMod>::operator %(int e)
{
    return v % e;
}

template< int TMod>
resring<TMod>::operator int_type() const
{
    return v;
}

template< int TMod>
void resring<TMod>::set(resring<TMod>::int_type val)
{
    v = mod(val);
}

template< int TMod>
typename resring<TMod>::int_type resring<TMod>::mod(resring<TMod>::int_type val)
{
    return ( val % TMod + 2 * TMod ) % TMod;
}

template< int TMod>
bool resring<TMod>::operator ==(const resring<TMod> &y) const
{
    return v == y.v;
}

template< int TMod>
bool resring<TMod>::operator !=(const resring &y) const
{
    return v != y.v;
}

//// template compilation request

template class resring<8>;
