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

#include "vector3hpr.h"

typedef Vector3hpr::mpclass mpclass;

Vector3hpr::Vector3hpr() :
    v{0,0,0}
{}

Vector3hpr::Vector3hpr( const mpclass& x, const mpclass &y, const mpclass& z )
{
    v[0] = x;
    v[1] = y;
    v[2] = z;
}

Vector3hpr Vector3hpr::cross( const Vector3hpr& a )
{
    return Vector3hpr( v[1]* a.v[2] -v[2]* a.v[1],
                       v[2]* a.v[0] -v[0]* a.v[2],
                       v[0]* a.v[1] -v[1]* a.v[0] );
}

Vector3hpr& Vector3hpr::operator=( const Vector3hpr& val )
{
    v[0] = val.v[0];
    v[1] = val.v[1];
    v[2] = val.v[2];
    return *this;
}

Vector3hpr Vector3hpr::operator /( const mpclass& val )
{
    Vector3hpr res;
    res.v[0] =  v[0] /val;
    res.v[1] =  v[1] /val;
    res.v[2] =  v[2] /val;
    return res;
}

mpclass Vector3hpr::dot( const Vector3hpr& val )
{
    return v[0] * val.v[0] + v[1] * val.v[1] + v[2] * val.v[2];;
}

mpclass Vector3hpr::squaredNorm()
{
    return v[0] *v[0] + v[1] * v[1] + v[2] * v[2];
}

mpclass Vector3hpr::norm()
{
    return sqrt( v[0] *v[0] + v[1] * v[1] + v[2] * v[2] );
}
