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

#include "matrix2x2.h"
#include "resring.h"
#include "hprhelpers.h"

#include <iostream>
#include <limits>
#include <algorithm>

using namespace std;

typedef ring_int<int>::mpclass mpclass;

template < class TInt>
matrix2x2<TInt> matrix2x2<TInt>::operator*( const matrix2x2<TInt>& b ) const
{
    return matrix2x2<TInt>(
                b.d[0][0]* d[0][0]+b.d[1][0]* d[0][1], b.d[0][1]* d[0][0]+b.d[1][1]* d[0][1],
                      b.d[0][0]* d[1][0]+b.d[1][0]* d[1][1], b.d[0][1]* d[1][0]+b.d[1][1]* d[1][1],
                      b.de + de);
}

template < class TInt>
matrix2x2<TInt> matrix2x2<TInt>::operator*( const ring_int<TInt>& b ) const
{
    return matrix2x2<TInt>( b * d[0][0], b*d[0][1], b*d[1][0], b*d[1][1], de);
}

template < class TInt>
void matrix2x2<TInt>::mul_eq_w( int k )
{
    d[0][0].mul_eq_w(k);
    d[0][1].mul_eq_w(k);
    d[1][0].mul_eq_w(k);
    d[1][1].mul_eq_w(k);
}


template < class TInt>
matrix2x2<TInt>& matrix2x2<TInt>::div_eq_sqrt2_exp(int a)
{
    d[0][0].div_eq_sqrt2(a) ; d[0][1].div_eq_sqrt2(a);
    d[1][0].div_eq_sqrt2(a) ; d[1][1].div_eq_sqrt2(a);
    return *this;
}

template < class TInt>
void matrix2x2<TInt>::set( const ring_int<TInt>& z, const ring_int<TInt>& u, const ring_int<TInt>& w, const ring_int<TInt>& v, int denom_exp )
{
    d[0][0] = z; d[0][1] = u;
    d[1][0] = w; d[1][1] = v;
    de = denom_exp;
}

template < class TInt>
matrix2x2<TInt>::matrix2x2( const ring_int<TInt>& z, const ring_int<TInt>& u, const ring_int<TInt>& w, const ring_int<TInt>& v, int denom_exp )
{
    set(z,u,
        w,v,
        denom_exp);
}

template < class TInt>
matrix2x2<TInt>::matrix2x2()
{
    set( ring_int<TInt>(),ring_int<TInt>(),ring_int<TInt>(),ring_int<TInt>(),0 );
}

template < class TInt>
int matrix2x2<TInt>::min_gde() const
{
    int min_gde = std::numeric_limits<int>::max();

    for( int i = 0; i < 2; ++i )
        for( int j = 0; j < 2; ++j )
            min_gde = std::min( d[i][j].gde(), min_gde );

    return min_gde;
}

template < class TInt>
int matrix2x2<TInt>::min_gde_abs2() const
{
    int min_gde = std::numeric_limits<int>::max();

    for( int i = 0; i < 2; ++i )
        for( int j = 0; j < 2; ++j )
            min_gde = std::min( d[i][j].abs2().gde(), min_gde );

    return min_gde;
}

template < class TInt>
int matrix2x2<TInt>::reduce()
{
    matrix2x2<TInt>& A = *this;
    int gde = min_gde();
    if( gde != std::numeric_limits<int>::max() )
    {
        int red_power = std::min ( gde, de );
        A.div_eq_sqrt2_exp(red_power);
        de -= red_power;
    }
    return de;
}


template < class TInt>
int matrix2x2<TInt>::reduce(int red_power)
{
    div_eq_sqrt2_exp(red_power);
    de -= red_power;
    return de;
}

template < class TInt >
int matrix2x2<TInt>::max_sde_abs2() const
{
    return sde( 2 * de, min_gde_abs2() );
}



template < class TInt >
void matrix2x2<TInt>::mul_TkH(int k, matrix2x2<TInt> &out)
{
    ring_int<TInt> d01wk;
    ring_int<TInt> d11wk;
    d[0][1].mul_w(k,d01wk);
    d[1][1].mul_w(k,d11wk);

    out.d[0][0] = d[0][0];
    out.d[0][0] += d01wk;

    out.d[1][0] = d[1][0];
    out.d[1][0] += d11wk;

    out.d[0][1] = d[0][0];
    out.d[0][1] -= d01wk;

    out.d[1][1] = d[1][0];
    out.d[1][1] -= d11wk;

    out.de = de + 1;
}

template < class TInt >
matrix2x2<TInt> matrix2x2<TInt>::T( int power )
{
    return matrix2x2<TInt>( ring_int<TInt>(1), ring_int<TInt>(0),
                      ring_int<TInt>(0), ring_int<TInt>::omega(power) );
}

template < class TInt>
matrix2x2<TInt> matrix2x2<TInt>::H()
{
    return matrix2x2<TInt>( ring_int<TInt>(1), ring_int<TInt>( 1),
                      ring_int<TInt>(1), ring_int<TInt>(-1),
                      1 );
}

template < class TInt>
matrix2x2<TInt> matrix2x2<TInt>::X()
{
    return matrix2x2<TInt>( ring_int<TInt>(0), ring_int<TInt>( 1),
                      ring_int<TInt>(1), ring_int<TInt>(0),
                      0 );
}

template < class TInt>
matrix2x2<TInt> matrix2x2<TInt>::Z()
{
    return matrix2x2<TInt>( ring_int<TInt>(1), ring_int<TInt>( 0),
                      ring_int<TInt>(0), ring_int<TInt>(-1),
                      0 );
}

template < class TInt>
matrix2x2<TInt> matrix2x2<TInt>::Y()
{
    return matrix2x2<TInt>( ring_int<TInt>(0), -ring_int<TInt>::omega(2),
                      ring_int<TInt>::omega(2), ring_int<TInt>(0),
                      0 );
}

template < class TInt>
matrix2x2<TInt> matrix2x2<TInt>::Id( int power )
{
    return matrix2x2<TInt>( ring_int<TInt>::omega(power), ring_int<TInt>(0),
                            ring_int<TInt>(0), ring_int<TInt>::omega(power),
                      0 );
}

template < class TInt>
matrix2x2<TInt> matrix2x2<TInt>::P()
{
    return matrix2x2<TInt>( ring_int<TInt>(1), ring_int<TInt>(0),
                      ring_int<TInt>(0), ring_int<TInt>::omega(2),
                      0 );
}

template < class TInt>
matrix2x2<TInt> matrix2x2<TInt>::conjugateTranspose()
{
    return matrix2x2<TInt>( d[0][0].conjugate(), d[1][0].conjugate(),
                      d[0][1].conjugate(), d[1][1].conjugate(), de );
}

template < class TInt>
bool matrix2x2<TInt>::is_unitary() const
{
    auto r1 = d[0][0].abs2() + d[0][1].abs2();
    auto r2 = d[1][0].abs2() + d[1][1].abs2();
    auto r3 = d[0][0].abs2() + d[1][0].abs2();
    auto r4 = d[0][1].abs2() + d[1][1].abs2();
    decltype(r1) sum( 1 );
    sum <<= de;
    return (r1 == sum) && (r2 == sum) && (r3 == sum) && (r4 == sum);
}

template < class TInt>
matrix2x2<TInt> &matrix2x2<TInt>::operator =(const vector2<TInt> &v)
{
    d[0][0] = v[0]; d[0][1] = - v[1].conjugate();
    d[1][0] = v[1]; d[1][1] = v[0].conjugate();
    de = v.de;
}

template < class TInt>
bool matrix2x2<TInt>::operator <( const matrix2x2<TInt>& B ) const
{
    if ( de > B.de )
        return false;
    else if( de < B.de )
        return true;

    for( int i = 0; i < 2; ++i )
        for( int j = 0; j < 2; ++j )
        {
            int le = d[i][j].le(B.d[i][j]); // +1 -- less, 0 -- equal, -1 greater
            if( le < 0 )
                return false;
            else if( le > 0 )
                return true;
        }
    return false;
}

template < class TInt >
template < class Tcl >
matrix2x2<TInt>::matrix2x2( const matrix2x2<Tcl>& val )
{
    typedef ring_int<TInt> r;
    set( (r) val.d[0][0], (r) val.d[0][1],(r) val.d[1][0],(r) val.d[1][1], val.de );
}

template < class TInt>
bool matrix2x2<TInt>::operator ==( const matrix2x2<TInt>& B ) const
{
  for( int i = 0; i < 2; ++i )
      for( int j = 0; j < 2; ++j )
      {
          if ( d[i][j].le(B.d[i][j]) != 0 ) // +1 -- less, 0 -- equal, -1 greater
            return false;
      }
  return true;
}


template < class TInt >
matrix2x2hpr::matrix2x2hpr(const matrix2x2<TInt> &val)
{
    for( int i = 0; i < 2; ++i )
        for( int j = 0; j < 2; ++j )
        {
            d[i][j] = val.d[i][j].toHprComplex( val.de );
        }
}

matrix2x2hpr::matrix2x2hpr( scalar a, scalar b, scalar c, scalar _d)
{
    d[0][0] = a;
    d[0][1] = b;
    d[1][0] = c;
    d[1][1] = _d;
}

matrix2x2hpr::matrix2x2hpr( cd a, cd b, cd c, cd _d)
{
    d[0][0] = a;
    d[0][1] = b;
    d[1][0] = c;
    d[1][1] = _d;
}

matrix2x2hpr::matrix2x2hpr( const matrix2x2cd& val )
{
    d[0][0] = val[0][0];
    d[0][1] = val[0][1];
    d[1][0] = val[1][0];
    d[1][1] = val[1][1];
}

matrix2x2hpr::matrix2x2hpr() :
    d( {
{scalar(mpclass(0),mpclass(0)),scalar(mpclass(0),mpclass(0))},
{scalar(mpclass(0),mpclass(0)),scalar(mpclass(0),mpclass(0))}}
       )
{
}

matrix2x2hpr::matrix2x2hpr( const matrix2x2hpr& val )
{
  *this = val;
}

matrix2x2hpr::scalar matrix2x2hpr::trace() const
{
    return d[0][0] + d[1][1];
}

matrix2x2hpr matrix2x2hpr::operator-( const matrix2x2hpr& b ) const
{
    matrix2x2hpr res;
    for( int i = 0; i < 2; ++i )
        for( int j = 0; j < 2; ++j )
        {
            res.d[i][j] = d[i][j] - b.d[i][j];
        }
    return res;
}

double matrix2x2hpr::dist(const matrix2x2hpr& matr) const
{
    mpclass dist(0);
    for( int i = 0 ; i < 2; ++i )
        for( int j = 0; j < 2; ++j )
        {
            mpclass d2 = std::norm( matr.d[i][j] - d[i][j] );
            dist += d2;
        }
    return hprHelpers::toMachine( sqrt(dist) );
}

const matrix2x2hpr& matrix2x2hpr::Id()
{
    static matrix2x2hpr Idm(1,0,
                           0,1);
    return Idm;
}

const matrix2x2hpr& matrix2x2hpr::X()
{
    static matrix2x2hpr Xm(0,1,
                          1,0);
    return Xm;
}

const matrix2x2hpr &matrix2x2hpr::Y()
{
    static matrix2x2hpr Ym(cd(0,0),cd(0,-1),
                           cd(0,1),cd(0, 0));
    return Ym;
}

const matrix2x2hpr &matrix2x2hpr::Z()
{
    static matrix2x2hpr Zm(1,0,
                          0,-1);
    return Zm;
}

matrix2x2hpr matrix2x2hpr::operator*( const matrix2x2hpr& b ) const
{
    return matrix2x2hpr(
                b.d[0][0]* d[0][0]+b.d[1][0]* d[0][1], b.d[0][1]* d[0][0]+b.d[1][1]* d[0][1],
                      b.d[0][0]* d[1][0]+b.d[1][0]* d[1][1], b.d[0][1]* d[1][0]+b.d[1][1]* d[1][1]);
}

matrix2x2hpr matrix2x2hpr::operator+( const matrix2x2hpr& b ) const
{
    matrix2x2hpr res;
    for( int i = 0; i < 2; ++i )
        for( int j = 0; j < 2; ++j )
        {
            res.d[i][j] = d[i][j] + b.d[i][j];
        }
    return res;
}

matrix2x2hpr matrix2x2hpr::operator*( const scalar& val ) const
{
    matrix2x2hpr res;
    for( int i = 0; i < 2; ++i )
        for( int j = 0; j < 2; ++j )
        {
            res.d[i][j] = d[i][j] * val;
        }
    return res;
}

matrix2x2hpr matrix2x2hpr::operator*( const mpclass& val ) const
{
    matrix2x2hpr res;
    for( int i = 0; i < 2; ++i )
        for( int j = 0; j < 2; ++j )
        {
            res.d[i][j] = d[i][j] * val;
        }
    return res;
}

matrix2x2hpr operator*( const mpclass& val, const matrix2x2hpr& rhs )
{
    return rhs * val;
}

matrix2x2hpr operator*( const matrix2x2hpr::scalar& val, const matrix2x2hpr& rhs )
{
    return rhs * val;
}

matrix2x2hpr& matrix2x2hpr::operator= ( const matrix2x2hpr& v )
{
    for( int i = 0; i < 2; ++i )
        for( int j = 0; j < 2; ++j )
        {
            d[i][j]=v.d[i][j];
        }
    return *this;
}

matrix2x2hpr matrix2x2hpr::adjoint() const
{
    return matrix2x2hpr( std::conj( d[0][0] ), std::conj( d[1][0]),
                      std::conj( d[0][1]), std::conj( d[1][1]) );
}

//////// template compilation requests //////////////////

template class matrix2x2<int>;
template class matrix2x2<long int>;
template class matrix2x2<mpz_class>;
template class matrix2x2< resring<8> >;

template matrix2x2hpr::matrix2x2hpr( const matrix2x2<int> &val );
template matrix2x2hpr::matrix2x2hpr( const matrix2x2<long int> &val );
template matrix2x2hpr::matrix2x2hpr( const matrix2x2<mpz_class> &val );

template matrix2x2<int>::matrix2x2( const matrix2x2<mpz_class>& );
template matrix2x2<long>::matrix2x2( const matrix2x2<mpz_class>& );
template matrix2x2<mpz_class>::matrix2x2( const matrix2x2<int>& );
template matrix2x2<mpz_class>::matrix2x2( const matrix2x2<long>& );
template matrix2x2< resring<8> >::matrix2x2( const matrix2x2<mpz_class>& );

//////////////// Helper functions //////////////////////

double trace_dist( const matrix2x2hpr& a, const matrix2x2hpr& b )
{
    static const mpclass& half = hprHelpers::half();
    static const mpclass& one = hprHelpers::one();

    // One could use Tailor expansion of the whole formula when a and b close to each other.
    // This would give better results in terms of precision.
    // We, intstead, take advantage of high precision arithmetic and use straightforward calculation

    return hprHelpers::toMachine( sqrt(one - abs( ( a * b.adjoint() ).trace() ) * half) );
}

void convert(const matrix2x2hpr& in, matrix2x2cd &out)
{
    for( int i =0; i < 2; ++i )
        for( int j =0; j < 2; ++j )
            hprHelpers::convert( in.d[i][j],out[i][j] );
}
