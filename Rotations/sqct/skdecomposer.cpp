//     Copyright (c) 2013 Jeff Heckey jheckey(at)ece(dot)ucsb(dot)edu
//     This file uses SQCT, Copyright (c) 2012 Vadym Kliuchnikov, Dmitri Maslov, Michele Mosca;
//     SQCT is distributed under LGPL v3
//

#include "skdecomposer.h"
#include "output.h"

#include <fstream>
#include <sstream>
#include <chrono>
#include <cmath>
#include <boost/timer/timer.hpp>

#ifndef SKD_H
#define SKD_H

namespace btm = boost::timer;

SKDecompose::SKDecompose()
{
	r.num = 1.0;
}

/// \brief Summarize gate statistics for Clifford + T library
void SKDecompose::updateForCliffordT()
{
	typedef gateLibrary gl;
	total = c.cost();
	auto counts = gl::toCliffordT( c.count() );
	hc = counts[ gl::H ];
	tc = counts[ gl::T ];
	pc = counts[ gl::P ];
	plc =  counts[ gl::Z ] + counts[ gl::X ] + counts[ gl::Y ];
}

/// \brief Finds circuit and all supplementary information for unitary
void SKDecompose::calculate( const matrix2x2hpr& matrix, int recursion_level )
{
	// boost::timer::cpu_times ct;
	// boost::timer::cpu_times ct2;

	sk::Me res;
    skd.decompose(matrix,res,recursion_level);

	int gde_before = res.min_gde_abs2();
	res.reduce();
	int gde_after = res.min_gde_abs2();
	denom_reduction = gde_before - gde_after;
	denom = res.max_sde_abs2();
	exact_uni = res;

    ed.decompose(res,c);

	sk::Ma conv(res);

	dst = trace_dist(matrix,conv);
	// tappr = (double) ct.wall * 1e-9;
	// tdecomp = (double) ct2.wall * 1e-9;
	nr = recursion_level;
	updateForCliffordT();
}

void SKDecompose::rotation( double angle, int iter, bool frac )
{
	static const double TwoPi = 2. * hprHelpers::toMachine( hprHelpers::pi() );
	if ( frac ) {
		r.num = 1.0;
		r.den = angle;
	} else {
		r.num = angle;
		r.den = TwoPi;
	}
	calculate( r.matrix() , iter );
}

/// \brief generate a X rotation
circuit SKDecompose::rotX( double angle, int iter, bool frac )
{
	r.nx = 1;
	r.ny = 0;
	r.nz = 0;
	rotation( angle, iter, frac );
	return c;
}

/// \brief generate a Y rotation
circuit SKDecompose::rotY( double angle, int iter, bool frac )
{
	r.nx = 0;
	r.ny = 1;
	r.nz = 0;
	rotation( angle, iter, frac );
	return c;
}

/// \brief generate a Z rotation
circuit SKDecompose::rotZ( double angle, int iter, bool frac )
{
	r.nx = 0;
	r.ny = 0;
	r.nz = 1;
	rotation( angle, iter, frac );
	return c;
}

#endif // SKD_H

