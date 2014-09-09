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

#include "hoptimalitytest.h"
#include "output.h"
#include "exactdecomposer.h"
#include "numbers-stat.h"

#include <iostream>
#include <iomanip>
using namespace std;


static void print_list( std::vector<int>& a, std::vector<int>& b, std::vector<int>& c)
{
    cout << "sde(|.|^2)   total   min-H   max-H" << endl;
    cout << "                     gates   gates" << endl;

    for( int i = 0; i < a.size(); ++i )
        if( a[i] != 0 )
            cout <<  setw (10) << i << ":" <<
                     setw (7) << a[i] << " " <<
                     setw (7) << b[i] << " " <<
                     setw (7) << c[i] << endl;
}

hoptimalitytest::hoptimalitytest()
{
    init();
}

//assumes that initial node is identity matrix
static void getCircuit( circuit& res, const optNode* node, const vector<int>& genIdToGateId )
{
    const optNode* current = node;
    while( current->parent != 0 )
    {
        res.push_front( genIdToGateId[current->gen_id] );
        current = current->parent;
    }
}

void hoptimalitytest::init()
{
    const bool verbose  = true;

    static gateLibrary gl;
    typedef matrix2x2<int> m;
    ogc.m_generators = { m::X(), m::Y(), m::Z(), m::T(), m::T().conjugateTranspose(), m::P(), m::P().conjugateTranspose() };
    vector<int> genIdToGateId = { gl.X, gl.Y, gl.Z, gl.T, gl.Td, gl.P, gl.Pd };
    ogc.m_cost = { 1,2,1,40,40,10,10 };
    ogc.m_initial = { m::Id() };
    vector<int> initialGates = { gl.Id };
    ogc.m_initial_cost = {0};
    ogc.generate();

    //produce couning bounds
    typedef columnsCounter<8> cC;
    unique_ptr<cC> cp( new cC );
    cp->generate_all_numbers();
    cp->count_all_columns();

    //setup bounds
    for( int i = 0 ; i < cp->sde_stat.size() ; ++i )
        og.m_sde_required.at(i) = cp->sde_stat[i];

    m_clifford.resize( ogc.unique_elements().size() );
    auto k = m_clifford.begin();

    for( auto i : ogc.unique_elements() )
    {
        getCircuit(*k,i.get(),genIdToGateId);
        if( verbose ) { k->toStreamSym(cout); cout << "," << endl; }

        og.m_generators.push_back( i->unitary * m::H() );
        og.m_cost.push_back( 1 );
        og.m_initial.push_back( i->unitary );
        ++k;
    }

    og.m_initial_cost.resize( og.m_initial.size(), 0 );
    og.generate();
    if( verbose ) print_list( og.m_sde_found, og.m_min_cost, og.m_max_cost );
}

