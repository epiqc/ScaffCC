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

#include "costs.hpp"

#include <boost/tuple/tuple.hpp>

#include "../target_tags.hpp"
#include "../functions/flatten_circuit.hpp"

namespace revkit
{

  cost_t gate_costs::operator()( const circuit& circ ) const
  {
    return circ.num_gates();
  }

  cost_t line_costs::operator()( const circuit& circ ) const
  {
    return circ.lines();
  }

  quantum_costs::quantum_costs()
    : controls_offset( 0 )
  {
  }

  cost_t quantum_costs::operator()( const gate& g, unsigned lines ) const
  {
    cost_t costs = 0ull;

    unsigned n = lines;
    unsigned c = std::distance( g.begin_controls(), g.end_controls() );

    if ( is_fredkin( g ) )
    {
      costs = 2ull;
      c += 1u;
    }

    c += std::max( -1 * (int)c, controls_offset );
    c = std::min( c, lines - 1u );

    unsigned e = n - c - 1u; // empty lines

    switch ( c ) {
    case 0u:
    case 1u:
      costs = 1ull;
      break;
    case 2u:
      costs = 5ull;
      break;
    case 3u:
      costs = 13ull;
      break;
    case 4u:
      costs = ( e >= 2u ) ? 26ull : 29ull;
      break;
    case 5u:
      if ( e >= 3u ) {
        costs = 38ull;
      } else if ( e >= 1u ) {
        costs = 52ull;
      } else {
        costs = 61ull;
      }
      break;
    case 6u:
      if ( e >= 4u ) {
        costs = 50ull;
      } else if ( e >= 1u ) {
        costs = 80ull;
      } else {
        costs = 125ull;
      }
      break;
    case 7u:
      if ( e >= 5u ) {
        costs = 62ull;
      } else if ( e >= 1u ) {
        costs = 100ull;
      } else {
        costs = 253ull;
      }
      break;
    case 8u:
      if ( e >= 6u ) {
        costs = 74ull;
      } else if ( e >= 1u ) {
        costs = 128ull;
      } else {
        costs = 509ull;
      }
      break;
    case 9u:
      if ( e >= 7u ) {
        costs = 86ull;
      } else if ( e >= 1u ) {
        costs = 152ull;
      } else {
        costs = 1021ull;
      }
      break;
    default:
      if ( e >= c - 2u ) {
        costs = 12ull * c - 33ull;
      } else if ( e >= 1u ) {
        costs = 24ull * c - 87ull;
      } else {
        costs = ( 1ull << ( c + 1ull ) ) - 3ull;
      }
    }

    return costs;
  }

  cost_t transistor_costs::operator()( const gate& g, unsigned lines ) const
  {
    return 8ull * std::distance( g.begin_controls(), g.end_controls() );
  }

  struct costs_visitor : public boost::static_visitor<cost_t>
  {
    explicit costs_visitor( const circuit& circ ) : circ( circ ) {}

    cost_t operator()( const costs_by_circuit_func& f ) const
    {
      // flatten before if the circuit has modules
      if ( circ.modules().empty() )
      {
        return f( circ );
      }
      else
      {
        circuit flattened;
        flatten_circuit( circ, flattened );
        return f( flattened );
      }
    }

    cost_t operator()( const costs_by_gate_func& f ) const
    {
      cost_t sum = 0ull;
      foreach ( const gate& g, circ )
      {
        // respect modules
        if ( is_module( g ) )
        {
          sum += costs( *boost::any_cast<module_tag>( g.type() ).reference.get(), f );
        }
        else
        {
          sum += f( g, circ.lines() );
        }
      }
      return sum;
    }

  private:
    const circuit& circ;
  };

  cost_t costs( const circuit& circ, const cost_function& f )
  {
    return boost::apply_visitor( costs_visitor( circ ), f );
  }

}
