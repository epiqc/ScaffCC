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

#ifndef GATELIBRARY_H
#define GATELIBRARY_H

#include <vector>
#include <string>
#include <deque>
#include <iostream>
#include <map>
#include "matrix2x2.h"

/// \brief Contains information about gates, e.g. corresponding strings etc
class gateLibrary
{
public:
    /// \name Gate constants
    /// @{
    static const int Id = 0; ///< Identity (\f$ I \f$ )
    static const int T = 1;  ///< \f$T\f$ gate, corresponding unitary is \f$ \left(\begin{array}{cc} 1 & 0\\ 0 & e^{i\frac{\pi}{4}} \end{array}\right) \f$
    static const int P = 2;  ///< Phase gate (\f$ P=T^2 \f$)
    static const int TP = 3; ///< \f$T^3\f$ or \f$ T P \f$
    static const int Z = 4;  ///< Pauli Z gate (\f$ Z=T^4 \f$)
    static const int TZ = 5; ///< \f$T^5\f$ or \f$ T Z \f$
    static const int Pd = 6; ///< Inverse of Phase gate (\f$P^{\dagger}=T^6 \f$)
    static const int Td = 7; ///< Inverse of T gate (\f$ T^{\dagger}=T^7 \f$)
    static const int H = 9;  ///< Hadamard gate, unitary is \f$ \frac{1}{\sqrt{2}}\left(\begin{array}{cc} 1 & 1\\1 & -1 \end{array}\right) \f$
    static const int X = 10; ///< Pauli X gate (\f$ HZH \f$)
    static const int Y = 11; ///< Pauli Y gate (\f$ iXZ \f$)
    static const int GLw1 = 12; ///< Global phase (\f$ \omega I, \omega =  e^{i\frac{\pi}{4}} \f$ )
    static const int GLw2 = 13; ///< Global phase (\f$ \omega^2 I \f$ )
    static const int GLw3 = 14; ///< Global phase (\f$ \omega^3 I \f$ )
    static const int GLw4 = 15; ///< Global phase (\f$ \omega^4 I \f$ )
    static const int GLw5 = 16; ///< Global phase (\f$ \omega^5 I \f$ )
    static const int GLw6 = 17; ///< Global phase (\f$ \omega^6 I \f$ )
    static const int GLw7 = 18; ///< Global phase (\f$ \omega^7 I \f$ )
    /// @}

    /// \brief Type of matrices used to store gates
    typedef matrix2x2<mpz_class> m;
    /// \brief Gate names, name[gate_id] returns the name of the gate
    std::vector<std::string > name;
    /// \brief Dot qc friendly names of gates
    std::vector<std::string > name_qc;
    /// \brief Mathematica friendly names of gates
    std::vector<std::string > matrix_str;
    /// \brief Matrices corresponding to each gate
    std::vector< m > matrix;
    /// \brief Map between symbols corresponding to gates and there id's
    std::map<char,int> symbols;
    /// \brief Identifiers corresponding to inverse gates
    std::vector< int > inverse;
    /// \brief Cost of each gate
    std::vector< int > cost;
    /// \brief Fills all information about gates
    gateLibrary();

    /// \brief Instance of gate library
    static const gateLibrary& instance();

    /// \brief Converts a general cost vector to cost vector over Clifford and T library.
    /// Combines cost of T and Td, P and Pd under T and P entries correspondingly.
    /// \see circuit class and circuit::count() function for more details
    static std::vector<int> toCliffordT( const std::vector<int>& val);
};

/// \brief Represents circuit
class circuit : public std::deque<int>
{
public:
    /// \brief Matrix type when circuit converted to unitary
    typedef matrix2x2<mpz_class> m;
    /// \brief Conversion operator to unitary
    operator matrix2x2<mpz_class> () const;
    /// \brief Converts circuit to unitary and writes result into res
    void convert( m& res ) const;
    /// \brief Outputs circuit to stream in Dot QC friendly way
    void toStream( std::ostream& out ) const;
    /// \brief Outputs circuit to string in Scaffold friendly way
    std::string toString() const;
    /// \brief Outputs circuit to stream using name vector of gateLibrary
    void toStreamSym( std::ostream &out) const;
    /// \brief Outputs string that is product of matrices corresponding
    /// to operation performed by circuit, in Mathematica format
    void toMathStream( std::ostream& out ) const;
    /// \brief Concatenetes circuit from the beginning
    void push_front( const circuit& c );
    /// \brief Concatenetes gate from the beginning
    void push_front( int gateId );
    /// \brief Concatenetes circuit from the end
    void push_back( const circuit& c );
    /// \brief Concatenetes gate from the end
    void push_back( int gateId);
    /// \brief Returns vector with number of gates with each gateId.
    /// For resulting vector \b res, the number \b res[gateId] gives a number of gates with
    /// specific id in the circuit.
    /// \see gateLibrary class for the list of ids
    std::vector<int> count();
    /// \brief Reads circuit from stream in line by line mode. Lines beginning with #
    /// are skipped. All charachters that are not gate symbols or '*','d' are ignored.
    /// If gate symbol is followed by * or d it repplaced by its inverse.
    void fromStream( std::istream& in, bool reverse = false );
    /// \brief Returns total cost of the circuit defined by gateLibrary::cost vector
    int cost();
};
#endif // GATELIBRARY_H
