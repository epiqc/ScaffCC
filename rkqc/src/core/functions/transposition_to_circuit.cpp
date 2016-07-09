#include "transposition_to_circuit.hpp"

#include <boost/assign/std/set.hpp>

#include "../circuit.hpp"
#include "../gate.hpp"

#include "add_circuit.hpp"
#include "add_gates.hpp"
#include "reverse_circuit.hpp"

using namespace boost::assign;

namespace revkit
{
  bool transposition_to_circuit( circuit& circ, const boost::dynamic_bitset<>& inputs, const boost::dynamic_bitset<>& outputs )
  {
    assert( inputs.size() == outputs.size() );
    assert( circ.lines() == inputs.size() );

    unsigned b = 0u, bs = 0u;
    circuit circ_block_a( inputs.size() );
    circuit circ_block_b( inputs.size() );
    circuit circ_block_bs( inputs.size() );
    circuit circ_block_c( inputs.size() );
    circuit circ_block_X( inputs.size() );
    append_not( circ_block_X, 0u );
    gate::line_container controls;
    unsigned target;

    for ( unsigned j = 0u; j < inputs.size(); ++j )
    {
      target = inputs.size() - 1u - j;
      if ( !inputs.test( j ) && !outputs.test( j ) )
      {
        append_not( circ_block_a, target );
      }
      else if ( inputs.test( j ) && !outputs.test( j ) )
      {
        controls = gate::line_container();
        append_not( circ_block_b, target );
        ++b;
        append_not( circ_block_c, target );
        for ( unsigned k = 0u; k < inputs.size(); ++k )
        {
          if ( k != target )
          {
            controls += k;
          }
        }
        append_toffoli( circ_block_c, controls, target );
        circ_block_X.remove_gate_at( 0u );
        append_toffoli( circ_block_X, controls, target );
      }
      else if ( !inputs.test( j ) && outputs.test( j ) )
      {
        controls = gate::line_container();
        append_not( circ_block_bs, target );
        ++bs;
        append_not( circ_block_c, target );
        for ( unsigned k = 0u; k < inputs.size(); ++k )
        {
          if ( k != target )
          {
            controls += k;
          }
        }
        append_toffoli( circ_block_c, controls, target );
        circ_block_X.remove_gate_at( 0u );
        append_toffoli( circ_block_X, controls, target );
      }
    }

    circ_block_c.remove_gate_at( circ_block_c.num_gates() - 1 );
    circ_block_c.remove_gate_at( circ_block_c.num_gates() - 1 );

    append_circuit( circ, circ_block_a );
    if ( b < bs )
    {
      append_circuit( circ, circ_block_b );
    }
    else
    {
      append_circuit( circ, circ_block_bs );
    }
    append_circuit( circ, circ_block_c );

    append_circuit( circ, circ_block_X );

    reverse_circuit( circ_block_c );
    append_circuit( circ, circ_block_c );

    if ( b < bs )
    {
      append_circuit( circ, circ_block_b );
    }
    else
    {
      append_circuit( circ, circ_block_bs );
    }
    append_circuit( circ, circ_block_a );

    return true;
  }
}
