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

#include "seqlookupcliff.h"
#include "output.h"
#include "numbersgen.h"
#include "gatelibrary.h"

#include <memory>
#include <iostream>

using namespace std;


static void print_list( std::vector<int>& a, std::vector<int>& b, std::vector<int>& c)
{
    for( int i = 0; i < a.size(); ++i )
        if( a[i] != 0 )
            cout << i << ":" << a[i] << " " << b[i] << " " << c[i] << endl;
}

seqLookupCliff::seqLookupCliff(bool full_init)
{
    if( full_init ) init();
}

void seqLookupCliff::find(const matrix2x2<int> &m, circuit &res)
{
    std::shared_ptr<optNode> val( new optNode({0,0,m,0}) );

    //search for matrix first
    auto el = og.unique_elements().find(val);
    for( int i = 1; i < 8 && (el == og.unique_elements().end()) ; ++i )
    {
        val->unitary.mul_eq_w(1);
        el = og.unique_elements().find(val);
    }

    res.clear();
    // compose circuit for it uising circuits for clifford unitaries and search tree data
    if( el != og.unique_elements().end() )
    {
        const optNode* current = el->get();
        int szu = ogc.unique_elements().size();
        while( current->parent != 0 )
        {
            res.push_front( m_clifford[current->gen_id] );
            if( current->gen_id >= szu )
                res.push_front( gateLibrary::Td );
            else
                res.push_front( gateLibrary::T );
            current = current->parent;
        }
        res.push_front( m_clifford[current->gen_id] );
    }
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

void seqLookupCliff::init()
{
    const bool verbose  = false;

    static gateLibrary gl;
    typedef matrix2x2<int> m;
    ogc.m_generators = { m::X(), m::Y(), m::Z(), m::H(), m::P(), m::P().conjugateTranspose() };
    vector<int> genIdToGateId = { gl.X, gl.Y, gl.Z, gl.H, gl.P, gl.Pd };
    ogc.m_cost = { 1,1,1,10,40,40 };
    ogc.m_initial = { m::Id() };
    vector<int> initialGates = { gl.Id };
    ogc.m_initial_cost = {0};
    ogc.generate();

    //produce couning bounds
    typedef columnsCounter<4> cC;
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
        if( verbose ) { k->toStreamSym(cout);cout << "," << endl; }

        og.m_generators.push_back( i->unitary * m::T() );
        og.m_cost.push_back( i->cost + 1000 ); // this garauntees T optimality and small counts
                                               // for other gates in the same time

        og.m_initial.push_back( i->unitary );
        og.m_initial_cost.push_back( i->cost );
        ++k;
    }

    for( auto i : ogc.unique_elements() )
    {
        og.m_generators.push_back( i->unitary * m::T(7) );
        og.m_cost.push_back( i->cost + 1000 ); // this garauntees T optimality and small counts
                                               // for other gates in the same time
    }

    og.generate();

    if( verbose ) print_list( og.m_sde_found, og.m_min_cost, og.m_max_cost );
}

