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

#include "theoremverification.h"
#include "rint.h"
#include "resring.h"

#include <vector>

bool is_theorem_true()
{
    // rring8 -- ring of integer residues modulo 8
    // ri8 -- extension of rring8 using eigth root of unity
    typedef ring_int< rring8 > ri8;
    std::vector<ri8> g[2][8][8];
    int d_conjecture[4] = {0,2,1,1};

    for( int x1 = 0; x1 < 8; ++x1 )
    for( int x2 = 0; x2 < 8; ++x2 )
    for( int x3 = 0; x3 < 8; ++x3 )
    for( int x4 = 0; x4 < 8; ++x4 )
    {
        ri8 x(x1,x2,x3,x4);
        auto j = x.abs2().gde();
        int a = (int) x.ipxx();
        int b = (int) x.ipQxx();
        if( j <= 1 )
            g[j][a][b].push_back(x);
    }

    for( int j = 0; j < 2; ++j )
    for( int ax = 0; ax < 8; ++ax )
    for( int bx = 0; bx < 8; ++bx )
    {
        int ay = rring8::mod(-ax);
        int by = rring8::mod(-bx);
        int d_count[4];

        for( auto x : g[j][ax][bx] )
        for( auto y : g[j][ay][by] )
        for( int d = 1; d <=3; ++d )
        {
            d_count[d] = 0;
            for( int k = 0; k < 4; ++k )
            {
                ri8 t = x + ri8::omega(k) * y;
                if ( t.abs2().gde() == d + j )
                    d_count[d]++;
            }

            if( d_conjecture[d] != d_count[d] )
                return false;
        }
    }

    return true;
}
