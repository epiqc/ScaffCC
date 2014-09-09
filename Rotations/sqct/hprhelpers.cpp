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

#define MPFR_REAL_DATA_PUBLIC
#include "hprhelpers.h"

void hprHelpers::convert(const hprHelpers::hpr_complex &in, std::complex<double> &out)
{
    out = std::complex<double>( mpfr_get_d( in.real()._x , MPFR_RNDN) , mpfr_get_d( in.imag()._x , MPFR_RNDN) );
}

void hprHelpers::convert(const hprHelpers::hpr_real &from, double &to)
{
    to = mpfr_get_d( from._x , MPFR_RNDN);
}

double hprHelpers::toMachine(const hprHelpers::hpr_real &from)
{
    return mpfr_get_d( from._x , MPFR_RNDN);
}

std::complex<double> hprHelpers::toMachine(const hprHelpers::hpr_complex &in)
{
    return std::complex<double>( mpfr_get_d( in.real()._x , MPFR_RNDN) , mpfr_get_d( in.imag()._x , MPFR_RNDN) );
}

/// \brief Holds string for Pi computed with high precision
struct piData
{
    hprHelpers::hpr_real pi;///< Pi
    piData()
    {
        pi = std::string( "3.14159265358979323846264338327950288419716939937510582097494459230781640628620899862803482534211706798"
	"2148086513282306647093844609550582231725359408128481117450284102701938521105559644622948954930381964428810975665933446128475648233786"
	"78316527120190914564856692346034861045432664821339360726024914127372458700660631558817488152092096282925409171536436789259036001133053054882"
	"04665213841469519415116094330572703657595919530921861173819326117931051185480744623799627495673518857527248912279381830119491298336733"
	"62440656643086021394946395224737190702179860943702770539217176293176752384674818467669405132000568127145263560827785771342757789609173637"
	"1787214684409012249534301465495853710507922796892589235420199561121290219608640344181598136297747713099605187072113499999983729780499510597317"
	"328160963185950244594553469083026425223082533446850352619311881710100031378387528865875332083"
	"8142061717766914730359825349042875546873115956286388235378759375195778185778053217122680661300192787661119590921642"
	"0198938095257201065485863279" );
    }
};

const hprHelpers::hpr_real &hprHelpers::pi()
{
    static piData d;
    return d.pi;
}

const hprHelpers::hpr_real &hprHelpers::one()
{
    static hpr_real m_one(1.0);
    return m_one;
}

const hprHelpers::hpr_real &hprHelpers::two()
{
    static hpr_real m_two(2.0);
    return m_two;
}

const hprHelpers::hpr_real &hprHelpers::half()
{
    static hpr_real m_half( one() / two() );
    return m_half;
}

const hprHelpers::hpr_real &hprHelpers::mhalf()
{
    static hpr_real m_mhalf( - one() / two() );
    return m_mhalf;
}

const hprHelpers::hpr_real &hprHelpers::sqrt2ov2()
{
    static hpr_real m_s( sqrt(two()) / two() );
    return m_s;
}
