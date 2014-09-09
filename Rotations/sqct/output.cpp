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

#include "output.h"
#include "resring.h"

template < int Tmod >
std::ostream& operator<<(std::ostream& out, const resring<Tmod>& x )
{
  out << "res" << Tmod << "["<< ( typename resring<Tmod>::int_type) x << "]";
  return out;
}

std::ostream& operator<<(std::ostream& out, const std::pair< std::complex<double>, std::complex<double> > & A )
{
    out << "{" << A.first.real() << "+ I " << A.first.imag() << "," << A.second.real() << "+ I " << A.second.imag() << "}" ;
    return out;
}

std::ostream& operator<<(std::ostream& out, const matrix2x2hpr& A )
{
    out << "matrix2x2hpr[{{" << A.d[0][0] <<","<< A.d[0][1] <<"}," <<std::endl;
    out << " {" << A.d[1][0] <<","<< A.d[1][1] <<"}}]";
    return out;
}

template < class TInt >
std::ostream& operator<<(std::ostream& out, const ring_int<TInt>& x )
{
  out << "ringInt[{";
  for( int i = 0 ; i < 4 ; ++i )
    if( i != 3 )
      out << x[i] << ",";
    else
      out << x[i] << "}]";
  return out;
}

template < class TInt >
std::ostream& operator<<(std::ostream& out, const vector2<TInt>& v )
{
  out << "vector2[{" << v.de << "," ;
  out << v.d[0] << "," << v.d[1] << "}]";
  return out;
}

template < class TInt >
std::ostream& operator<<(std::ostream& out, const matrix2x2<TInt>& A )
{
  out << "# matrix2x2[{{" << A.d[0][0] <<","<< A.d[0][1] <<"}," <<std::endl;
  out << "#            {" << A.d[1][0] <<","<< A.d[1][1] <<"}," << A.de << "}]";
  return out;
}

template < class TInt >
std::istream& operator>>(std::istream& in, ring_int<TInt>& A )
{
    for( int i = 0; i < 4; ++i )
        in >> A[i];
    return in;
}

template < class TInt >
std::istream& operator>>(std::istream& in, matrix2x2<TInt>& A )
{

    for( int i = 0; i < 2; ++i )
        for( int j = 0; j < 2; ++j )
            in >> A.d[i][j];
    in >> A.de;
    return in;
}


//// templates compilation requests

template std::ostream& operator<<(std::ostream& out, const matrix2x2<int>& A );
template std::ostream& operator<<(std::ostream& out, const matrix2x2<long int>& A );
template std::ostream& operator<<(std::ostream& out, const matrix2x2<mpz_class>& A );
template std::ostream& operator<<(std::ostream& out, const matrix2x2<rring8>& x );

template std::ostream& operator<<(std::ostream& out, const vector2<int>& A );
template std::ostream& operator<<(std::ostream& out, const vector2<long int>& A );
template std::ostream& operator<<(std::ostream& out, const vector2<mpz_class>& A );
template std::ostream& operator<<(std::ostream& out, const vector2<rring8>& x );

template std::ostream& operator<<(std::ostream& out, const ring_int<int>& x );
template std::ostream& operator<<(std::ostream& out, const ring_int<long int>& x );
template std::ostream& operator<<(std::ostream& out, const ring_int<mpz_class>& x );
template std::ostream& operator<<(std::ostream& out, const ring_int<rring8>& x );

template std::istream& operator>>(std::istream& in, ring_int<mpz_class>& A );
template std::istream& operator>>(std::istream& in, ring_int<int>& A );
template std::istream& operator>>(std::istream& in, ring_int<long int>& A );

template std::istream& operator>>(std::istream& in, matrix2x2<mpz_class>& A );
template std::istream& operator>>(std::istream& in, matrix2x2<int>& A );
template std::istream& operator>>(std::istream& in, matrix2x2<long int>& A );

