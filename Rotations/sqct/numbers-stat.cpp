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

#include "numbers-stat.h"

template< int n>
Stats<n>::Stats()
{
    base::generate_all_numbers();
    static const int sde_range = base::denom_exp * 2 + 1;
    vector<int> sde_stat_has_pair;
    vector<int> sde_stat_no_pair;
    vector<int> max_solutions;
    vector<int> min_solutions;
    sde_stat_has_pair.resize(sde_range);
    sde_stat_no_pair.resize(sde_range);
    max_solutions.resize(sde_range);
    min_solutions.resize(sde_range, numeric_limits<int>::max());

    for( int ipxx = 0; ipxx < base::max_val2 + 1; ++ipxx )
    {
        for( int j = 0; j < base::ipQ_range; ++j )
        {
            int ipyy = base::max_val2 - ipxx;
            int ipQxx = j - base::ipQ_offset;
            int ipQyy = -ipQxx;
            int k = base::ipQ_offset + ipQyy;

            int sde = base::sdes[ipxx][j];
            int sde2 = base::sdes[ipyy][k];

            int szx = base::vals[ipxx][j].size();
            int szy = base::vals[ipyy][k].size();

            int idx = min(sde2,sde);

            if( szx > 0 && szy > 0 )
            {
                sde_stat_has_pair[idx] ++;
                max_solutions[idx]  = max( max_solutions[idx],szx );
                min_solutions[idx]  = min( min_solutions[idx],szx );
            }

            if( szx > 0 && szy == 0)
            {
                sde_stat_no_pair[sde] ++;
            }
        }
    }

    int s1 = sde_stat_has_pair[0] ;
    int s2 = sde_stat_no_pair[0] ;
    for( int i = 0; i < sde_range; ++i )
    {
        cout << i << " " << s1 << " " << s2 << " " <<  (double) s1 / (double) (s1 + s2 ) << " " << min_solutions[i] << " " << max_solutions[i]  << endl;
        s1 = s1 + sde_stat_has_pair[i];
        s2 = s1 + sde_stat_has_pair[i];
    }
}

///////////// Compilation request ////////////////////
template class Stats<2>;
template class Stats<4>;
template class Stats<6>;
template class Stats<8>;
template class Stats<10>;
template class Stats<12>;


template< int n>
void numbersStatistics()
{
    auto st = shared_ptr<Stats<n> > (new Stats<n>);
}

///////////// Compilation request ////////////////////
template void numbersStatistics<2>();
template void numbersStatistics<4>();
template void numbersStatistics<6>();
template void numbersStatistics<8>();
template void numbersStatistics<10>();
template void numbersStatistics<12>();

