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

#include "exactdecomposer.h"
#include "matrix2x2.h"
#include "output.h"

#include <vector>
#include <iostream>
#include <deque>

using namespace std;

exactDecomposer::exactDecomposer()
{
    for( int i = 0; i < gen_count ; ++i)
    {
        generators[i] = M::T(8-i) * M::H() ;
        generators_ctr[i] = M::H() * M::T(i);
    }
}

void exactDecomposer::decompose( const matrix2x2<mpz_class>& matr, circuit& c)
{
    typedef ring_int< resring<8> > rr8;
    typedef matrix2x2< resring<8> > mrr8;

    c.clear();
    M current(matr);
    current.reduce();
    M t;

    int curr_sde = current.max_sde_abs2();
    while( curr_sde > 3 )
    {
        bool found = false;
        mrr8 mr( current ), tr; //the only last 8 bits matter
        int curr_gde = mr.d[0][0].abs2().gde();

        for( int i = 0; i < gen_count; ++i )
        {

            mr.mul_TkH(8-i,tr);
            int gde_got = tr.d[0][0].abs2().gde();
            if( gde_got == curr_gde + 3 )
            {
                curr_sde--;
                c.push_back( i );
                c.push_back( gateLibrary::H );

                current.mul_TkH(8-i,t);

                if( gde_got == 3 )
                    t.reduce(1);
                else
                    t.reduce(2);

                current = t;
                found = true;
                break;
            }
        }
        if( !found ) return;
    }

    circuit r;
    slC.find(current,r);
    c.push_back(r);
}
