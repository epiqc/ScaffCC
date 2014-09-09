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

#include "optsequencegenerator.h"
#include <stdexcept>

using namespace std;

optSequenceGenerator::optSequenceGenerator()
{
}

bool optSequenceGenerator::processMatrix(const optNode &val, int counter)
{
    return false;
}

void optSequenceGenerator::generate()
{
    if( m_generators.size() != m_cost.size() )
        throw logic_error("cost and generators must have the same size");
    if( m_initial_cost.size() != m_initial.size() )
        throw logic_error("initial and initial cost must have the same size");

    //initialize search
    for( int i = 0; i < m_initial.size(); ++i )
    {
        nodeptr val ( new optNode({i,0,m_initial[i], m_initial_cost[i]}) );
        auto res = m_set.insert( val );
        if( res.second )
        {
            m_pq.push( val.get() );
            processMatrix(*val,1);
        }
    }

    int counter = 0;
    bool terminate = false;
    while ( ! m_pq.empty() && ! terminate )
    {
        auto top = m_pq.top();
        m_pq.pop();


        for( int i = 0; i < m_generators.size(); ++i )
        {
            counter++;
            nodeptr val ( new optNode(
                                  {i,
                                   top,
                                   m_generators[i] * top->unitary,
                                   m_cost[i] + top->cost } )
                        );

            val->unitary.reduce();

            int count = 0;
            for( int j = 0; j < 8; ++j )
            {
                val->unitary.mul_eq_w(1);
                if( m_set.count(val) != 0 )
                {
                    count++;
                    break;
                }
            }

            if( count == 0 ) // we haven't find unitary before
            {
                m_set.insert(val);
                m_pq.push(val.get());
                terminate = processMatrix( *val, counter);
            }
        }
    }
}

optSequenceGenerator::~optSequenceGenerator()
{
}

const optSequenceGenerator::nset &optSequenceGenerator::unique_elements() const
{
    return m_set;
}

bool optSequenceGeneratorSdeLim::processMatrix(const optNode &val, int counter)
{
    static const int verbose = false;
    if( verbose )
    {
        if( counter % 10000 == 0 )
        {
            for( int i = 0; i < m_sde_found.size(); ++i )
                if( m_sde_found[i] != 0 )
                    cout << i << ":" <<  m_sde_found[i] << endl;
        }
    }

    int sde = val.unitary.max_sde_abs2();
    m_sde_found[ sde ]++;
    m_min_cost[sde] = min ( m_min_cost[sde], val.cost );
    m_max_cost[sde] = max ( m_max_cost[sde], val.cost );

    for( int i = 0; i < max_sde; ++i )
    {
        if( m_sde_required[i] != 0 )
        {
            if( m_sde_found[i]  < m_sde_required[i] )
                return false;
            if( m_sde_found[i]  > m_sde_required[i] )
                throw new std::logic_error("m_sde_required must be maximal possible");
        }
    }
    return true;
}
