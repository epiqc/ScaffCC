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

#include "rint.h"
#include "resring.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <algorithm>
#include <cmath>

#include <iostream>

typedef ring_int<int>::mpclass mpclass;

static const double SQRT2_2 =0.70710678118654752440084436210484903928483593768847;

typedef mpz_class mpz_arr4[4];
std::complex<double> toComplex( const mpz_arr4& val, int de);

typedef int int_arr4[4];
std::complex<double> toComplex( const int_arr4& val, int de);

typedef long long_arr4[4];
std::complex<double> toComplex( const long_arr4& val, int de);

typedef rring8 rring8_arr4[4];
std::complex<double> toComplex( const rring8_arr4& val, int de);

std::complex<mpclass> toHprComplex( const mpz_arr4& val, int de);
std::complex<mpclass> toHprComplex( const int_arr4& val, int de);
std::complex<mpclass> toHprComplex( const long_arr4& val, int de);
std::complex<mpclass> toHprComplex( const rring8_arr4& val, int de);

/// \brief Precomuted powers of \f$ \frac{1}{\sqrt{2}} \f$
struct PrecDenom
{
    static const int max_val = 1000;
    double sqrt2inv[max_val];
    PrecDenom()
    {
       sqrt2inv[0] = 1.0;
       for( int i = 1; i < max_val; ++i )
           sqrt2inv[i] = sqrt2inv[i-1] * SQRT2_2;
    }
};

std::complex<double> toComplex( const int_arr4& v, int de)
{
    static const PrecDenom dd;
    double real = (double)v[0] + SQRT2_2 * (double) (v[1] - v[3]);
    long double img = (double)v[2] + SQRT2_2 * (double) (v[1] + v[3]);
    return std::complex<double>(real * dd.sqrt2inv[de], img * dd.sqrt2inv[de] );
}

inline void toComplex( const int_arr4& v, int de, double& re, double& im)
{
    static const PrecDenom dd;
    re = dd.sqrt2inv[de] * ( (double)v[0] + SQRT2_2 * (double) (v[1] - v[3]) );
    im = dd.sqrt2inv[de] * ( (double)v[2] + SQRT2_2 * (double) (v[1] + v[3]) );
}

inline void toComplex( const long_arr4& v, int de, double& re, double& im)
{
    static const PrecDenom dd;
    re = dd.sqrt2inv[de] * ( (double)v[0] + SQRT2_2 * (double) (v[1] - v[3]) );
    im = dd.sqrt2inv[de] * ( (double)v[2] + SQRT2_2 * (double) (v[1] + v[3]) );
}

inline void toComplex( const rring8_arr4& v, int de, double& re, double& im)
{
    assert( !"Not implemented" );
    throw std::exception();
}

inline void toComplex( const mpz_arr4& val, int de, double& re, double& im)
{
    assert( !"Not implemented" );
    throw std::exception();
}

std::complex<double> toComplex( const long_arr4& v, int de)
{
    static const PrecDenom dd;
    double real = (double)v[0] + SQRT2_2 * (double) (v[1] - v[3]);
    long double img = (double)v[2] + SQRT2_2 * (double) (v[1] + v[3]);
    return std::complex<double>(real * dd.sqrt2inv[de], img * dd.sqrt2inv[de] );
}


std::complex<double> toComplex( const mpz_arr4& val, int de)
{
    int denom_power2 = de / 2;
    bool mul_by_sqrt_2 = de % 2 == 1;
    mpz_class denom(1);
    double arr[4];
    denom <<= denom_power2;
    for( int i = 0; i < 4; ++i )
    {
        mpq_class af(val[i],denom);
        af.canonicalize();
        arr[i] = af.get_d();
    }

    if( mul_by_sqrt_2 ) // need to divide by sqrt2 once more
    {
        for( int i =0; i < 4; ++i )
            arr[i] *= SQRT2_2;
    }

    double real = (double)arr[0] + SQRT2_2 * (double) (arr[1] - arr[3]);
    double img = (double)arr[2] + SQRT2_2 * (double) (arr[1] + arr[3]);

    return std::complex<double>(real,img);
}

std::complex<double> toComplex( const rring8_arr4& val, int de)
{
    assert( !"Not implemented: operation is meaningless" );
}

std::complex<mpclass> toHprComplex( const mpz_arr4& val, int de)
{
    static const mpclass& sqrt2ov2 = hprHelpers::sqrt2ov2();
    int denom_power2 = de / 2;
    bool mul_by_sqrt_2 = de % 2 == 1;
    mpz_class denom(1);
    mpclass arr[4] = { mpclass(0.0),mpclass(0.0),mpclass(0.0),mpclass(0.0)};
    denom <<= denom_power2;
    for( int i = 0; i < 4; ++i )
    {
        mpclass dn = denom.get_str();
        mpclass v = val[i].get_str();
        arr[i] =  v / dn;
    }

    if( mul_by_sqrt_2 ) // need to divide by sqrt2 once more
    {
        for( int i =0; i < 4; ++i )
            arr[i] *= sqrt2ov2;
    }

    mpclass real = arr[0] + sqrt2ov2 * (arr[1] - arr[3]);
    mpclass img = arr[2] + sqrt2ov2 * (arr[1] + arr[3]);

    return std::complex<mpclass>(real,img);
}

std::complex<mpclass> toHprComplex( const int_arr4& val, int de)
{
    std::complex<double> r = toComplex(val,de);
    return std::complex<mpclass>(r.real(),r.imag());
}

std::complex<mpclass> toHprComplex( const long_arr4& val, int de)
{
    std::complex<double> r = toComplex(val,de);
    return std::complex<mpclass>(r.real(),r.imag());
}

std::complex<mpclass> toHprComplex( const rring8_arr4& val, int de)
{
    assert( !"Not implemented: operation is meaningless" );
}

int sde( int denom_exponent, int gde )
{
  if ( gde != std::numeric_limits<int>::max() )
    return  denom_exponent - gde;
  else
    return gde;
}

template < class T >
ring_int<T>::ring_int()
{
    set(0,0,0,0);
}

template< class TInt>
template<class T>
ring_int<TInt>::ring_int( const ring_int<T>& val )
{
    set( (TInt) val[0], (TInt) val[1], (TInt) val[2], (TInt) val[3]);
}

template<>
template<>
ring_int<int>::ring_int( const ring_int<mpz_class>& val )
{
    typedef int TInt ;
    set( (TInt) val[0].get_si(), (TInt) val[1].get_si(), (TInt) val[2].get_si(), (TInt) val[3].get_si() );
}

template<>
template<>
ring_int<long int>::ring_int( const ring_int<mpz_class>& val )
{
    typedef long int TInt ;
    set( (TInt) val[0].get_si(), (TInt) val[1].get_si(), (TInt) val[2].get_si(), (TInt) val[3].get_si() );
}

template < class T >
ring_int<T>::ring_int ( int_type a, int_type b, int_type c, int_type d)
{
    set(a,b,c,d);
}

template < class T >
ring_int<T>::ring_int ( int_type a )
{
    set(a,0,0,0);
}

template < class T >
ring_int<T> ring_int<T>::canonical() const
{
    static const double PI_8 = 0.39269908169872415480783042290993786052464617492189;
    static const double PI_4inv = 1.2732395447351626861510701069801148962756771659237;
    static const double PI_4 = 0.78539816339744830961566084581987572104929234984378;
    static const double PI2 = 6.2831853071795864769252867665590057683943387987502;
    std::complex<double> val( toComplex(0) );
    auto ar = std::arg( val ) + PI2;
    int omega_power = std::floor( ar * PI_4inv );
    double res = ar - PI_4 * omega_power;
    ring_int<T> r( *this );
    r.mul_eq_w( -omega_power );
    if( res >= PI_8 )
    {
        r.mul_eq_w(-1);
        r.conjugate_eq();
    }
    return r;
}

std::complex<double> canonical( const std::complex<double>& val, int &w_power, bool &conj )
{
    static const double PI_8 = 0.39269908169872415480783042290993786052464617492189;
    static const double PI_4inv = 1.2732395447351626861510701069801148962756771659237;
    static const double PI_4 = 0.78539816339744830961566084581987572104929234984378;
    static const double PI2 = 6.2831853071795864769252867665590057683943387987502;
    static const double SQRT2 = 0.707106781186547524400844362105;

    auto ar = std::arg( val ) + PI2;
    int omega_power = std::floor( ar * PI_4inv );
    double res = ar - PI_4 * omega_power;
    auto wp = ring_int<int>::omega( -omega_power );
    w_power = omega_power;
    auto r = val * wp.toComplex(0);
    static std::complex< double> winv ( SQRT2, - SQRT2);

    if( res >= PI_8 )
    {
        r *= winv;
        r = std::conj( r );
        conj = true;
        w_power += 1;
    }
    else
        conj = false;
    return r;
}

template < class T >
void ring_int<T>::set( int_type a, int_type b, int_type c, int_type d)
{
    v[0]=a; v[1]=b; v[2]=c; v[3]=d;
}

template < class T >
ring_int<T>& ring_int<T>::operator +=(const ring_int& y)
{
    v[0]+=y.v[0]; v[1]+=y.v[1]; v[2]+=y.v[2]; v[3]+=y.v[3];
    return *this;
}

template < class T >
ring_int<T>& ring_int<T>::operator -=(const ring_int& y)
{
    v[0]-=y.v[0]; v[1]-=y.v[1]; v[2]-=y.v[2]; v[3]-=y.v[3];
    return *this;
}

template < class T >
ring_int<T> ring_int<T>::operator +(const ring_int<T>& y) const
{
    return ring_int( v[0]+y.v[0], v[1]+y.v[1], v[2]+y.v[2], v[3]+y.v[3]);
}

template < class T >
ring_int<T> ring_int<T>::operator -(const ring_int<T>& y) const
{
    return ring_int( v[0]-y.v[0], v[1]-y.v[1], v[2]-y.v[2], v[3]-y.v[3]);
}

template < class T >
ring_int<T> ring_int<T>::operator -() const
{
    return ring_int( -v[0], -v[1], -v[2], -v[3] );
}


template < class T >
ring_int<T> ring_int<T>::operator *(const ring_int<T>& y) const
{
    return ring_int(v[0]* y.v[0] - v[3]* y.v[1] - v[2]* y.v[2] - v[1]* y.v[3],
                v[1]* y.v[0] + v[0]* y.v[1] - v[3]* y.v[2] - v[2]* y.v[3],
                v[2]* y.v[0] + v[1]* y.v[1] + v[0]* y.v[2] - v[3]* y.v[3],
                v[3]* y.v[0] + v[2]* y.v[1] + v[1]* y.v[2] + v[0]* y.v[3] );
}

template < class T >
ring_int<T> ring_int<T>::operator /( int_type a) const
{
    return ring_int( v[0] / a, v[1] / a, v[2] / a, v[3] /a );
}

template < class T >
ring_int<T>& ring_int<T>::operator /=( int_type a)
{
    v[0] /= a ; v[1] /= a; v[2] /= a, v[3] /= a ;
    return *this;
}

template < class T >
ring_int<T>& ring_int<T>::operator <<=( long a )
{
    v[0] <<= a ; v[1] <<= a; v[2] <<= a, v[3] <<= a ;
    return *this;
}

template < class T >
ring_int<T> &ring_int<T>::operator >>=( long a )
{
    v[0] >>= a ; v[1] >>= a; v[2] >>= a, v[3] >>= a ;
    return *this;
}

template < class T >
const typename ring_int<T>::int_type& ring_int<T>::operator [](int id) const
{
    return v[id];
}

template < class T >
typename ring_int<T>::int_type &ring_int<T>::operator [](int id)
{
    return v[id];
}

template < class T >
ring_int<T> ring_int<T>::sqrt2()
{
    return ring_int(0,1,0,-1);
}

template < class T >
ring_int<T> ring_int<T>::conjugate() const
{
    return ring_int(v[0],-v[3],-v[2],-v[1]);
}

template < class T >
void ring_int<T>::conjugate_eq()
{
    set(v[0],-v[3],-v[2],-v[1]);
}

template < class T >
std::complex<double> ring_int<T>::toComplex(int de) const
{
    return ::toComplex( v, de);
}

template < class T >
void ring_int<T>::toComplex( int d, double& re, double& im ) const
{
    ::toComplex(v, d ,re, im );
}

template < class T >
std::complex<mpclass> ring_int<T>::toHprComplex(int de) const
{
    return ::toHprComplex( v, de);
}

template < class T >
ring_int<T>& ring_int<T>::operator =(const ring_int<T>& b)
{
    set( b.v[0], b.v[1], b.v[2], b.v[3] );
    return *this;
}

template < class T >
ring_int<T>::ring_int (const ring_int<T>& b)
{
    set( b.v[0], b.v[1], b.v[2], b.v[3] );
}


template < class T >
int ring_int<T>::gde() const
{
    const ring_int<T>& x = *this;

    int gde2_min = std::numeric_limits<int>::max(); // minimum of gde of a base 2 over all entries
    int gde_res = std::numeric_limits<int>::max(); // gde of base sqrt(2) of x

    for( int i = 0; i < 4; ++i )
        gde2_min = std::min( gde2_min, gde2(v[i]) );

    if( gde2_min != std::numeric_limits<int>::max() ) // at leas one entry is not 0
    {
        int_type div = 1;
        div <<= gde2_min; // 2^gde2_min
        ring_int<T> y = x / div * sqrt2();

        assert( y * div == x *  sqrt2() );

        if( y.is_div_2() ) // 2 divides sqrt(2) * x / div , therefore sqrt(2) divides x / div
            gde_res = gde2_min * 2 + 1;
        else
            gde_res = gde2_min * 2;
    }

    return gde_res;
}

template < class T >
void ring_int<T>::div_eq_sqrt2( int n )
{
    ring_int<T>& x = *this;
    int n2 = n / 2;
    x >>= n2;

    if( n % 2 == 1 )
    {
        mul_eq_sqrt2();
        x >>= 1;
    }
}

template < class T >
void ring_int<T>::div_eq_sqrt2()
{
    mul_eq_sqrt2();
    div_eq_2();
}

template < class T >
void ring_int<T>::div_eq_2()
{
    v[0]>>=1;  v[1]>>=1;
    v[2]>>=1;  v[3]>>=1;
}

template < class T >
void ring_int<T>::mul_eq_w(int k)
{
    k = ( k % 8 + 16 ) % 8;
    switch( k )
    {
    case 0:
        break;
    case 1:
        set( -v[3], v[0], v[1], v[2]);
        break;
    case 2:
        set( -v[2], -v[3], v[0], v[1]);
        break;
    case 3:
        set( -v[1], -v[2], -v[3], v[0]);
        break;
    case 4:
        set( -v[0], -v[1], -v[2], -v[3]);
        break;
    case 5:
        set( v[3], -v[0], -v[1], -v[2]);
        break;
    case 6:
        set( v[2], v[3], -v[0], -v[1]);
        break;
    case 7:
        set( v[1], v[2], v[3], -v[0]);
        break;
    }
}


template < class T >
void ring_int<T>::mul_w(int k, ring_int<T>& out ) const
{
    k = ( k % 8 + 16 ) % 8;
    switch( k )
    {
    case 0:
        out.set(v[0], v[1], v[2],v[3] );
        break;
    case 1:
        out.set( -v[3], v[0], v[1], v[2]);
        break;
    case 2:
        out.set( -v[2], -v[3], v[0], v[1]);
        break;
    case 3:
        out.set( -v[1], -v[2], -v[3], v[0]);
        break;
    case 4:
        out.set( -v[0], -v[1], -v[2], -v[3]);
        break;
    case 5:
        out.set( v[3], -v[0], -v[1], -v[2]);
        break;
    case 6:
        out.set( v[2], v[3], -v[0], -v[1]);
        break;
    case 7:
        out.set( v[1], v[2], v[3], -v[0]);
        break;
    }
}

template < class T >
void ring_int<T>::mul_eq_w()
{
    set( -v[3], v[0], v[1], v[2]);
}

template < class T >
void ring_int<T>::mul_eq_sqrt2()
{
    set( v[1] - v[3],  v[2] + v[0], v[1] + v[3], v[2] - v[0]);
}

template < class T >
int ring_int<T>::gde2 ( ring_int<T>::pr_type a )
{
    if( a == 0)
        return std::numeric_limits<int>::max();

    int res = 0;
    while( is_div_2(a) )
    {
        res ++ ;
        a /= 2;
    }
    return res;
}

template < class T >
int ring_int<T>::isGde1(ring_int<T>::pr_type a, ring_int<T>::pr_type b)
{
    if( b % 2 == 1 && a % 2 == 0 )
        return true;
    return false;
}

template < class T >
int ring_int<T>::isGde0(ring_int<T>::pr_type a, ring_int<T>::pr_type b)
{
    if( a % 2 == 1 )
        return true;
    return false;
}

template < class T >
int ring_int<T>::isGde2(ring_int<T>::pr_type a, ring_int<T>::pr_type b)
{
    if( b % 2 == 0 && a % 2 == 0 && (a >> 1) % 2 == 1 )
        return true;
}

template < class T >
bool ring_int<T>::is_compl(const ring_int<T> &v) const
{
    if( ipQxx() + v.ipQxx() == 0 )
    {
        int_type ssq = ipxx() + v.ipxx();
        int g2 = gde2( ssq );
        int_type val = 1;
        val <<= g2;
        return ssq == val;
    }
    else
        return false;
}

template < class T >
bool ring_int<T>::is_im_eq0() const
{
    return (v[2] == 0) && (v[1] == - v[3]);
}

template < class T >
bool ring_int<T>::is_div_2( pr_type a )
{
    return a % 2 == 0;
}

template < class T >
bool ring_int<T>::is_div_2() const
{
    return is_div_2( v[0] ) && is_div_2( v[1] ) &&
           is_div_2( v[2] ) && is_div_2( v[3] );
}

template < class T >
ring_int<T> ring_int<T>::omega( int power)
{
    power = ( power % 8 + 16 ) % 8;
    int k = 1;
    if( power >= 4 )
    {
        k = -1;
        power -= 4;
    }
    ring_int<T> res;
    res.v[ power ] = k;
    return res;
}

template < class T >
typename ring_int<T>::pr_type ring_int<T>::ipQxx() const
{
    typedef typename ring_int<T>::pr_type tp;
    tp a = v[0]; tp b = v[1]; tp c = v[2]; tp d = v[3];
    return a*b + b*c + c*d - a*d;
}

template < class T >
typename ring_int<T>::pr_type ring_int<T>::ipxx() const
{
    typedef typename ring_int<T>::pr_type tp;
    tp a = v[0]; tp b = v[1]; tp c = v[2]; tp d = v[3];
    return a*a + b*b + c*c + d*d;
}

template < class T >
ring_int_real< typename ring_int<T>::pr_type > ring_int<T>::abs2() const
{
    return ring_int_real< ring_int<T>::pr_type >(ipxx(),ipQxx() );
}

template < class T >
typename ring_int<T>::int_type ring_int<T>::max_dist(const ring_int<T> &y) const
{
    int_type md = 0;
    for( int i = 0; i < 4; ++i )
    {
        int_type d = abs( y.v[i] - v[i] );
        if( d > md ) md = d;
    }
    return md;
}

template < class T >
int ring_int<T>::le( const ring_int<T>& y ) const
{
    for( int i = 0; i < 4; ++i )
    {
        if( v[i] > y.v[i] )
            return -1;
        else if( v[i] < y.v[i] )
            return 1;
    }
    return 0;
}

template < class T >
bool ring_int<T>::operator <( const ring_int<T>& y ) const
{
    return  le(y) > 0;
}

template < class T >
bool ring_int<T>::operator ==(const ring_int<T> &y) const
{
    return le(y) == 0;
}

template < class T >
bool ring_int<T>::operator !=(const ring_int<T> &y) const
{
    return le(y) != 0;
}

template < class T >
ring_int_real<T>::ring_int_real(ring_int_real<T>::int_type a, ring_int_real<T>::int_type b)
{
    base::set(a,b,0,-b);
}

template < class T >
int ring_int_real<T>::gde() const
{
    int g = 0;
    if( base::v[0] == 0 && base::v[1] == 0 )
        return std::numeric_limits<int>::max();

    int ga = base::gde2( base::v[0] );
    int gb = base::gde2( base::v[1] );

    if( gb < ga )
        return (gb << 1) + 1;
    else
        return (ga << 1);

}

//////// template compilation requests

template class ring_int<int>;
template class ring_int<long int>;
template class ring_int<mpz_class>;
template class ring_int<rring8>;

template class ring_int_real<int>;
template class ring_int_real<long int>;
template class ring_int_real<mpz_class>;
template class ring_int_real<rring8>;

template ring_int<int>::ring_int( const ring_int<mpz_class>& val );
template ring_int<long int>::ring_int( const ring_int<mpz_class>& val );
template ring_int<mpz_class>::ring_int( const ring_int<int>& val );
template ring_int<mpz_class>::ring_int( const ring_int<long int>& val );
template ring_int< resring<8> >::ring_int( const ring_int<mpz_class>& val );
