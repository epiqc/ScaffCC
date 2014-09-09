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

#include "gcommdecomposer.h"
#include <complex>
#include <exception>
#include <cassert>
#include <iostream>
#include "output.h"
#include "rint.h"
#include "vector3hpr.h"
#include "hprhelpers.h"

using namespace std;

/// \brief Type for high precision real numbers
typedef ring_int<int>::mpclass mpclass;
/// \brief Type for high precision matrices
typedef matrix2x2hpr M;
/// \brief Type for high precision matrices
typedef Vector3hpr V;

/// \brief Returns rotation by angle \f$ \theta \f$ around axis V:
/// \f$ I\cos\left(\frac{\theta}{2}\right)-i\sin\left(\frac{\theta}{2}\right)\left(n_{x}X+n_{y}Y+n_{z}Z\right) \f$,
/// where X,Y,Z -- Pauli matrices
/// \param sinTheta2 \f$ sin(\theta / 2) \f$
M rot( const mpclass& sinTheta2, V& axis )
{
    static const mpclass& one = hprHelpers::one();
    mpclass cosTheta2 = sqrt( one - sinTheta2 * sinTheta2 );
    return  M::Id() * cosTheta2 + M::scalar(0,-sinTheta2) * ( axis.v[0] *M::X()  + axis.v[1]*M::Y() + axis.v[2]*M::Z() );
}

/// \brief Returns group comutator of rotations around X and Y axis by angle \f$ \theta \f$
M gc( const mpclass& sinTheta2 )
{
    static V x(1.,0.,0.);
    static V y(0.,1.,0.);
    M r1 = rot(sinTheta2,x);
    M r2 = rot(sinTheta2,y);
    return r1 * r2 * r1.adjoint() * r2.adjoint();
}

/// \brief Computes rotations by angle \f$ \theta \f$   around X and Y axis conjugated by M
/// \param sinTheta2 \f$ \sin(\theta / 2 ) \f$
void corrGC( const M& corr, mpclass sinTheta2, M &U, M &W )
{
    static V x(1.,0.,0.);
    static V y(0.,1.,0.);
    M r1 = rot(sinTheta2,x);
    M r2 = rot(sinTheta2,y);
    U = corr * r1 * corr.adjoint();
    W = corr * r2 * corr.adjoint();
}

/// \brief Computes rotation axes of input unitary and stores result
struct axisAngle
{
    /// \brief Type for high precision real numbers
    typedef ring_int<int>::mpclass mpclass;
    /// \brief Type for high precision matrices
    typedef matrix2x2hpr M;
    /// \brief Type for high precision matrices
    typedef Vector3hpr V;

    /// \brief Computes rotation axes of input unitary
    axisAngle( const M& U )
    {
        static const mpclass& mh = hprHelpers::mhalf();
        coeffs = V( mh * ( U * M::X() ).trace().imag(),
                    mh * ( U * M::Y() ).trace().imag(),
                    mh * ( U * M::Z() ).trace().imag() );
        s = coeffs.squaredNorm();
        axis = coeffs / sqrt(s);
    }

    V coeffs;   ///< Coefficients of matrix decomposition into Pauli basis
    mpclass s;  ///< The values \f$ s = \sin( \theta / 2 )^2 \f$, where \f$ \theta \f$ rotation angle of input unitary
    V axis;     ///< Rotation axis corresponding to input unitary
};

void GC::decompose(const M &U, M &Vr, M &W)
{
    //// step by step check !!!!!!!!!!!!
    static const mpclass& one = hprHelpers::one();
    static const mpclass& two = hprHelpers::two();
    static const mpclass& half = hprHelpers::half();

    axisAngle aaU(U);

    // Here we are solving equation (10) from section 4.1 of http://arxiv.org/abs/quant-ph/0505030
    // aaU.s = sin( \theta / 2 )^2
    mpclass min_root = half * ( one - sqrt(one - aaU.s));

    // p = sin( \phi /2 )
    mpclass p = sqrt( sqrt( min_root ));

    // Steps to find matrix S
    M gcxy = gc(p);
    axisAngle aaGC(gcxy);

    V cr = aaGC.axis.cross( aaU.axis );
    mpclass ip = aaGC.axis.dot( aaU.axis );
    mpclass sinTheta2 = sqrt( half *( one - ip ));
    V newAxis = cr / cr.norm();
    M S = rot( sinTheta2, newAxis ); //this corresponds to S matrix after equation (11)

    corrGC( S, p, Vr, W ); // Vr = \tilde{V}, W = \tilde{W} from the paper
}

///////////////////////////////////////////////////////////////////////

bool eq( double a, double b )
{
    if( fabs( a - b ) < 1e-6 )
        return true;
    return false;
}

matrix2x2hpr Rotation::matrix()
{
    static const mpclass& pi = hprHelpers::pi();
    mpclass denh(den);
    mpclass numh(num);
    mpclass ang = (pi * numh) / denh;
    mpclass r = mpfr::sin( ang );
    V vec( nx,ny,nz);
    V vec_n = vec / vec.norm(); //normalize to reduce rounding off errors
    return rot( r , vec_n );
}

string Rotation::symbolic() const
{
    stringstream circ_s;
    char tag = isSpecial();
    if( tag != 'N' )
    {
        int d = floor(den);
        int n = floor(num);
        if( n == 1 )
            circ_s << "Id*cos(Pi/" << d << ")-i*sin(Pi/" << d
                   << ")*" << tag;
        else
            circ_s << "Id*cos(" << n << "*Pi/" << d << ")-i*sin("
                   << n << "*Pi/" << d
                   << ")*" << tag;

    }
    else
        circ_s << "Id*cos(" << num << "*Pi/" << den << ")-i*sin(" << num << "*Pi/"
               << den << ")*(X*" << nx << "+Y*" << ny << "+Z*" << nz << ")";

    return circ_s.str();
}

string Rotation::CSV() const
{
    stringstream circ_s;
    circ_s << num << "," << den << "," << nx << "," << ny << "," << nz;
    return circ_s.str();
}

string Rotation::Mathematica() const
{
    stringstream circ_s;
    circ_s.precision(10);
    circ_s.setf( ios_base::fixed ); // use fixed as mathematica does not understand scientific c++ format
    circ_s << "Rot[" << num << "," << den << "," << nx << "," << ny << "," << nz << "]";
    return circ_s.str();
}

string Rotation::name() const
{
    stringstream circ_s;
    char tag = isSpecial();
    int d = floor(den);
    int n = floor(num);
    if( tag != 'N' )
        circ_s << "R" << tag << n << "d" << d ;
    else
        circ_s << "Rot";
    return circ_s.str();
}


char Rotation::isSpecial() const
{
    if( ! eq( den , floor(den) ) )
        return 'N';

    if( ! eq( num, floor(num) ) )
        return 'N';

    char tag = 'N';

    if( eq(nx,1.) && eq(ny,0.) && eq(nz,0.) )
        tag = 'X';
    if( eq(nx,0.) && eq(ny,1.) && eq(nz,0.) )
        tag = 'Y';
    if( eq(nx,0.) && eq(ny,0.) && eq(nz,1.) )
        tag = 'Z';

    return tag;
}
