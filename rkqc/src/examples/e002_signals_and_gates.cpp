/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2011  The RevKit Developers <revkit@informatik.uni-bremen.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>

#include <core/circuit.hpp>
#include <core/functions/add_gates.hpp>
#include <core/io/write_realization.hpp>
#include <core/io/print_circuit.hpp>
#include <boost/lexical_cast.hpp>

using namespace revkit;

int main( int argc, char ** argv )
{

    qbit a;
    qbit b;
    qbit c;
    qbit d(10);
	qbit e(10);
	qbit f(10);
/*------------ Gates Called By Qubit --------------*/
    NOT(a);
    cnot(a,b);
    toffoli(a, b, c);	
/*------------ Gates Called By Register Index -----*/ 
    NOT(d[0]);
    cnot(d[1], d[9]);
    toffoli(d[0], e[0], c[0]);
/*------------ Gates Called By Mixed Register Index & Qbits -----*/ 
    cnot(a, d[0]);
    cnot(e[0], b);
	toffoli(d[0], a, b);
	toffoli(a, d[0], b);
	toffoli(a, b, d[0]); 
	toffoli(d[0], e[0], a);
	toffoli(d[0], a, e[0]);
	toffoli(a, d[0], e[0]);
/*------------ Gates Called By Full Qbit Reg ------*/ 
	NOT(d);
	cnot(d, e);
	toffoli(d, e, f);

    return 0;
}
