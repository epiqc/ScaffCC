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

#include "sk.h"
#include "gcommdecomposer.h"
#include "output.h"

#include <iostream>
using namespace std;

sk::sk(int max_layer) :
    uapp( max_layer )
{
}

typedef ring_int<int>::mpclass mpclass;

void sk::decompose(const sk::Ma &U, sk::Me &out, int n)
{
    if( n == 0 )
    {
        indexedUnitaryApproximator::Ma tmp;
        convert( U,tmp );
        uapp.approximate(tmp,out);
    }
    else
    {
        Me Ue;
        decompose(U,Ue,n-1);
        Ma V,W;
        GC::decompose( U * Ma(Ue).adjoint() ,V,W);
        Me Ve,We;
        decompose(V,Ve,n-1);
        decompose(W,We,n-1);
        out = Ve * We * Ve.conjugateTranspose() * We.conjugateTranspose() * Ue;
    }
}
