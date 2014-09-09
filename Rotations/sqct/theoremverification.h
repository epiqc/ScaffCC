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

#ifndef THEOREMVERIFICATION_H
#define THEOREMVERIFICATION_H

/// \brief Verifies statement of the theorem about unitary decomposition
/// Proves Lemma 3 using Algorithm 2 in http://arxiv.org/abs/1206.5236
/// This is improved version, that proves that there two ways to increase
/// sde, one way to leave it the same and one way to decrease it if we
/// use \f$ H T^k, k \in \{0,1,2,3\} \f$
bool is_theorem_true();

#endif // THEOREMVERIFICATION_H
