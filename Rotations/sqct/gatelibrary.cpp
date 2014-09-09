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

#include "gatelibrary.h"
#include "output.h"

#include <sstream>

using namespace std;

gateLibrary::gateLibrary() :
    name(GLw7 + 1), name_qc(GLw7 + 1),
    matrix_str( GLw7 + 1), matrix( GLw7 + 1),
    inverse(GLw7 + 1), cost( GLw7 + 1,0 )
{
    name[Id] = "I";
    name[T] = "T";
    name[P] = "P";
    name[TP] = "tZ";
    name[Z] = "Z";
    name[TZ] = "TZ";
    name[Pd] = "p";
    name[Td] = "t";
    name[H] = "H";
    name[X] = "X";
    name[Y] = "Y";

    name_qc[Id] = "Id 1";
    name_qc[T] = "T 1";
    name_qc[P] = "P 1";
    name_qc[TP] = "T* 1\nZ 1";
    name_qc[Z] = "Z 1";
    name_qc[TZ] = "T 1\nZ 1";
    name_qc[Pd] = "P* 1";
    name_qc[Td] = "T* 1";
    name_qc[H] = "H 1";
    name_qc[X] = "X 1";
    name_qc[Y] = "Y 1";

    for( int i = 1; i < 8; i++ )
    {
        stringstream ss;
        ss << "GLw" << i ;
        name_qc[(GLw1 - 1 + i)] = ss.str();
        name[(GLw1 - 1 + i)] = ss.str();
    }

    matrix_str[Id] = "Id";
    matrix_str[T] = "T";
    matrix_str[P] = "P";
    matrix_str[TP] = "Td.Z";
    matrix_str[Z] = "Z";
    matrix_str[TZ] = "T.Z";
    matrix_str[Pd] = "Pd";
    matrix_str[Td] = "Td";
    matrix_str[H] = "H";
    matrix_str[X] = "X";
    matrix_str[Y] = "Y";

    for( int i = 1; i < 8; i++ )
    {
        stringstream ss;
        ss << "(omega^" << i <<"*Id)" ;
        matrix_str[(GLw1 - 1 + i)] = ss.str();
    }

    matrix[Id] = m::Id();
    matrix[T] = m::T(1);
    matrix[P] = m::P();
    matrix[TP] = m::T(3);
    matrix[Z] = m::Z();
    matrix[TZ] = m::T(5);
    matrix[Pd] = m::T(6);
    matrix[Td] = m::T(7);
    matrix[H] = m::H();
    matrix[X] = m::X();
    matrix[Y] = m::Y();

    for( int i = 1; i < 8; i++ )
        matrix[(GLw1 - 1 + i)] = m::Id(i);

    inverse[Id] = Id;
    inverse[T]  = Td;
    inverse[Td]  = T;
    inverse[P]  = Pd;
    inverse[Pd]  = P;
    inverse[TP] = TZ;
    inverse[TZ] = TP;

    inverse[Z] = Z;
    inverse[X] = X;
    inverse[Y] = Y;
    inverse[H] = H;

    for( int i = 0; i < 8; ++i )
        inverse[GLw1 + i ] = inverse[GLw1 + 7 - i ];

    symbols['T'] = T;
    symbols['t'] = Td;

    symbols['P'] = P;
    symbols['p'] = Pd;
    symbols['S'] = P;
    symbols['s'] = Pd;

    symbols['Z'] = Z;
    symbols['z'] = Z;

    symbols['X'] = X;
    symbols['x'] = X;

    symbols['Y'] = Y;
    symbols['y'] = Y;

    symbols['H'] = H;
    symbols['h'] = H;

    cost[Td] = cost[T] = 1000;
    cost[H] = 10;
    cost[P] = cost[Pd] = 40;
    cost[X] = cost[Z] = 1;
    cost[Y] = 2;
    cost[TP] = cost[Z] + cost[Td]; //interpret TP as Td Z
    cost[TZ] = cost[T] + cost[Z];
}

const gateLibrary &gateLibrary::instance()
{
    static gateLibrary gl;
    return gl;
}

vector<int> gateLibrary::toCliffordT(const vector<int> &val)
{
    auto res = val;
    res[T] += res[TZ];
    res[Z] += res[TZ];
    res[TZ] = 0;

    res[T] += res[TP]; //interpret TP as Td Z
    res[Z] += res[TP]; //interpret TP as Td Z
    res[TP] = 0;

    res[T] += res[Td];
    res[Td] = 0;

    res[P] += res[Pd];
    res[Pd] = 0;

    return res;
}

void circuit::convert(circuit::m& res) const
{
    const circuit& c = *this;
    static const gateLibrary& gl = gateLibrary::instance();
    res = gl.matrix[gl.Id];
    for( int i = 0; i < size(); ++i )
    {
       m tmp = gl.matrix[ c[i] ] * res;
       res = tmp;
       //cout << res << endl;
    }
    res.reduce();
}

circuit::operator matrix2x2<mpz_class> () const
{
    circuit::m res;
    convert(res);
    return res;
}

void circuit::toStream(ostream &out) const
{
    const circuit& c = *this;
    static const gateLibrary& gl = gateLibrary::instance();
    if( size() == 0 )
    {
        out << gl.name_qc[gl.Id] << endl;
        return;
    }

    for( int i = 0; i < size(); ++i )
        out << gl.name_qc[ c[i] ] << endl;
}

string circuit::toString() const
{
    stringstream ss;
    const circuit& c = *this;
    static const gateLibrary& gl = gateLibrary::instance();
    if( size() == 0 )
    {
        ss << gl.name[gl.Id] << endl;
        return ss.str();
    }

    for( int i = 0; i < size(); ++i )
        ss << gl.name[ c[i] ];
		return ss.str();
}

void circuit::toStreamSym(ostream &out) const
{
    const circuit& c = *this;
    static const gateLibrary& gl = gateLibrary::instance();

    if( size() == 0 )
    {
        out << gl.name[gl.Id] ;
        return;
    }

    for( int i = 0; i < size() - 1; ++i )
        out << gl.name[ c[i] ] << " ";
    out << gl.name[ c.back() ];
}

void circuit::toMathStream(ostream &out) const
{
    const circuit& c = *this;
    static const gateLibrary& gl = gateLibrary::instance();

    if( size() == 0 )
    {
        out << gl.matrix_str[gl.Id];
        return;
    }

    for( int i = size() - 1; i > 0 ; --i )
        out << gl.matrix_str[ c[i] ] << ".";
    out << gl.matrix_str[ c.front() ];
}

void circuit::push_front(const circuit &c)
{
    for( int i = c.size() - 1; i >= 0; --i )
        push_front(c[i]);
}

void circuit::push_front(int v)
{
    std::deque<int>::push_front(v);
}

void circuit::push_back( int v )
{
    std::deque<int>::push_back(v);
}

void circuit::push_back( const circuit& c )
{
    for( auto i : c )
        push_back(i);
}

vector<int> circuit::count()
{
    static const gateLibrary& gl = gateLibrary::instance();
    vector<int> res( gl.GLw7 + 1,0 );
    for( auto i : *this )
        res[i]++;

    return res;
}

void circuit::fromStream(istream &in, bool reverse)
{
    static const gateLibrary& gl = gateLibrary::instance();
    while(true)
    {
        string line;
        getline(in,line);
        if( !in )
            break;

        if ( reverse )
            for( char c : line )
            {
                if( gl.symbols.count(c) != 0 )
                    push_front(gl.symbols.find(c)->second);
                else if( c == '*' || c == 'd' )
                    front() = gl.inverse[ front() ];
            }
        else
            for( char c : line )
            {
                if( gl.symbols.count(c) != 0 )
                    push_back(gl.symbols.find(c)->second);
                else if( c == '*' || c == 'd' )
                    back() = gl.inverse[ back() ];
            }

    }
}

int circuit::cost()
{
    static const gateLibrary& gl = gateLibrary::instance();
    int sum = 0;
    for( int i = 0; i < size(); ++i )
        sum += gl.cost[ at(i) ];
    return sum;
}

