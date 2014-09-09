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

#ifndef NUMBERSGEN_H
#define NUMBERSGEN_H

#include "rint.h"
#include <vector>


/// \brief Exhaustively generates numbers in the ring that has pair. Code desined for even n.
/// \tparam n Power of \f$\sqrt{2}\f$ in the denominator of numbers
/// \note Class precompiled for n = 2,4,6,8,10,12
template< int n >
struct numbersGenerator
{
public:
    /// \brief Type to store found integeres in the ring \f$ \mathbb{Z}[\frac{1}{\sqrt{2}},i]\f$
    typedef ring_int<int> ri;
    /// \brief Type for collection of integeres in the ring \f$ \mathbb{Z}[\frac{1}{\sqrt{2}},i]\f$
    typedef std::vector<ri> riArray;

    /// \brief Performs basic initialization
    numbersGenerator();

    /// \brief Finds all numbers that has pair to makeunit column.
    ///
    /// Formally this is equivalent to the following property:
    /// \li there exists y -- integer in the ring  \f$ \mathbb{Z}[\frac{1}{\sqrt{2}},i]\f$ such that
    /// \f$ |x|^2 + |y|^2 = 2^n, \f$ where \f$ x = x_0 + x_1 \omega + x_2 \omega^2 + x_3 \omega^3 \f$,
    /// \f$ \omega = e^{i \frac{\pi}{4}}\f$.
    ///
    /// This implies following constraints that we take into account during generation process:
    /// \li \f$ x_k \le 2^{n/2}, k = 0,1,2,3\f$
    /// \li \f$ P(x) \le 4^{n/2} \f$
    /// \li \f$ Q(x) \le  4^{n/2} \f$
    /// \li \f$ \left|\frac{x}{\sqrt{2}^n}\right|^2 \le 1 \f$
    ///
    /// See Section 5 of http://arxiv.org/abs/1206.5236 for definitions of P(x) and Q(x).
    void generate_all_numbers();

    /// \brief Returns vector containing
    /// all numbers with P(x)=ipxx and Q(x)=ipQxx
    const riArray& numbers(int ipxx, int ipQxx ) const;

    /// \brief The same as numbers(), but returns empty vector
    /// if a number doesn't have a pair to make a unit column
    const riArray& numbersWithPair(int ipxx, int ipQxx ) const;

    /// \brief Returns \f$ sde(|\cdot|^2) \f$ of number x such that P(x)=ipxx and Q(x)=ipQxx
    int getSde( int ipxx, int ipQxx ) const;

    /// \brief Maximal denominator exponent
    static const int denom_exp = n;

    /// \brief Maximal denominator exponent of squares
    static const int max_sde = 2 * denom_exp;

    /// \brief Maximal absolute value of each integer coordinate
    static const int max_val = 1 << (denom_exp / 2);

    /// \brief Maximal absolute value of each integer coordinate squared
    static const int max_val2 = max_val * max_val;
    /// \brief Maximal value of P(x) + 1
    static const int ip_range = max_val2 + 1;
    /// \brief Number of different values that Q(x) can achieve + 1
    static const int ipQ_range = 2 * max_val2 + 1;
    /// \brief Q(x) belongs to interval [-ipQ_offset,ipQ_range- ipQ_offset + 1)
    static const int ipQ_offset = max_val2;
protected:
    /// \brief Type of array used to store found numbers
    typedef riArray  vals_arr[ ip_range ][ ipQ_range ];

    /// \brief Integers in the ring with P(x) = "first index"
    /// and Q(x) = "second index"
    vals_arr vals;

    typedef int sde_arr[ ip_range ][ ipQ_range ];
    /// \brief values of gde of elements of vals array
    sde_arr sdes;

    /// \brief How many numbers were generated
    int m_total_numbers;
private:
    /// \brief Adds nuber to collection of found numbers
    void add_number( int a, int b , int c, int d, int ipxx );
    /// \brief Initializes arrays
    void init();
    /// \brief Verifies if P(x) and Q(x) are in valid range
    bool isInRange( int ipxx, int ipQxx ) const;
};

/// \brief Computes and stores quantity of unit column with entries with specific \f$ sde(|\cdot|^2) \f$.
/// \note Class precompiled for n = 2,4,6,8,10,12
template< int n >
struct columnsCounter : public numbersGenerator<n>
{
    /// \brief Upper bound of \f$ sde(|\cdot|^2) \f$ considered
    static const int max_sde;
    /// \brief The value opf vector with index \f$k\f$ containts number of columns with \f$ sde(|\cdot|^2) = k \f$.
    std::vector<int> sde_stat;
    /// \brief Basic class type
    typedef numbersGenerator<n> bs;
    /// \brief Computes quantity of unit column with entries with specific \f$ sde(|\cdot|^2) \f$.
    void count_all_columns();
};

#endif // NUMBERSGEN_H
