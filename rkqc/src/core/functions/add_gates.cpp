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

#include "add_gates.hpp"
#include <fstream>

#include <boost/assign/std/vector.hpp>
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <boost/range/algorithm.hpp>

#include "../circuit.hpp"
#include "../target_tags.hpp"

using namespace boost::assign;

namespace revkit
{

  ////////////////////////////// class target_line_adder
  target_line_adder::target_line_adder( gate* gate ) : g( gate )
  {
  }

  gate& target_line_adder::operator()( const gate::line& l1 )
  {
    g->add_target( l1 );
    return *g;
  }

  gate& target_line_adder::operator()( const gate::line& l1, const gate::line& l2 )
  {
    g->add_target( l1 );
    g->add_target( l2 );
    return *g;
  }

  ////////////////////////////// class control_line_adder
  control_line_adder::control_line_adder( gate& gate ) : g( &gate )
  {
  }

  target_line_adder control_line_adder::operator()()
  {
    return target_line_adder( g );
  }

  target_line_adder control_line_adder::operator()( const gate::line& l1 )
  {
    g->add_control( l1 );
    return target_line_adder( g );
  }

  target_line_adder control_line_adder::operator()( const gate::line& l1, const gate::line& l2 )
  {
    g->add_control( l1 );
    g->add_control( l2 );
    return target_line_adder( g );
  }

  target_line_adder control_line_adder::operator()( const gate::line& l1, const gate::line& l2, const gate::line& l3 )
  {
    g->add_control( l1 );
    g->add_control( l2 );
    g->add_control( l3 );
    return target_line_adder( g );
  }

  target_line_adder control_line_adder::operator()( const gate::line& l1, const gate::line& l2, const gate::line& l3, const gate::line& l4 )
  {
    g->add_control( l1 );
    g->add_control( l2 );
    g->add_control( l3 );
    g->add_control( l4 );
    return target_line_adder( g );
  }

  target_line_adder control_line_adder::operator()( const gate::line& l1, const gate::line& l2, const gate::line& l3, const gate::line& l4, const gate::line& l5 )
  {
    g->add_control( l1 );
    g->add_control( l2 );
    g->add_control( l3 );
    g->add_control( l4 );
    g->add_control( l5 );
    return target_line_adder( g );
  }

  target_line_adder control_line_adder::operator()( const gate::line& l1, const gate::line& l2, const gate::line& l3, const gate::line& l4, const gate::line& l5, const gate::line& l6 )
  {
    g->add_control( l1 );
    g->add_control( l2 );
    g->add_control( l3 );
    g->add_control( l4 );
    g->add_control( l5 );
    g->add_control( l6 );
    return target_line_adder( g );
  }

  target_line_adder control_line_adder::operator()( const gate::line& l1, const gate::line& l2, const gate::line& l3, const gate::line& l4, const gate::line& l5, const gate::line& l6, const gate::line& l7 )
  {
    g->add_control( l1 );
    g->add_control( l2 );
    g->add_control( l3 );
    g->add_control( l4 );
    g->add_control( l5 );
    g->add_control( l6 );
    g->add_control( l7 );
    return target_line_adder( g );
  }

  target_line_adder control_line_adder::operator()( const gate::line& l1, const gate::line& l2, const gate::line& l3, const gate::line& l4, const gate::line& l5, const gate::line& l6, const gate::line& l7, const gate::line& l8 )
  {
    g->add_control( l1 );
    g->add_control( l2 );
    g->add_control( l3 );
    g->add_control( l4 );
    g->add_control( l5 );
    g->add_control( l6 );
    g->add_control( l7 );
    g->add_control( l8 );
    return target_line_adder( g );
  }

  target_line_adder control_line_adder::operator()( const gate::line& l1, const gate::line& l2, const gate::line& l3, const gate::line& l4, const gate::line& l5, const gate::line& l6, const gate::line& l7, const gate::line& l8, const gate::line& l9 )
  {
    g->add_control( l1 );
    g->add_control( l2 );
    g->add_control( l3 );
    g->add_control( l4 );
    g->add_control( l5 );
    g->add_control( l6 );
    g->add_control( l7 );
    g->add_control( l8 );
    g->add_control( l9 );
    return target_line_adder( g );
  }

  ////////////////////////////// create_ functions


  void set_LLVM(bool setter){
    LLVM_IR = setter;
  }

  void create_toffoli( circuit& circ, const gate::line& control1, const gate::line& control2, const gate::line& target )
  {
    std::ofstream gates;
    gates.open("gates.txt", std::ios_base::app);
    if (gates.is_open()){
        std::string in1 = circ.lines_to_inputs.find( control1 )->second;
        std::string in2 = circ.lines_to_inputs.find( control2 )->second;
        std::string in3 = circ.lines_to_inputs.find( target )->second;
        int count1 = registerCount++;
        int count2 = registerCount++;
        int count3 = registerCount++;
        if( LLVM_IR ) {
            gates << "  %" << count1 << " = load i16* %" << in1 << ", align 2\n"; 
            gates << "  %" << count2 << " = load i16* %" << in2 << ", align 2\n"; 
            gates << "  %" << count3 << " = load i16* %" << in3 << ", align 2\n"; 
            gates << "  call void @llvm.Toffoli( i16 %" << count1 << ", i16 %" << count2 << ", i16 %" << count3 << ")\n";

        }
        else {
            gates << "Toffoli (" << circ.lines_to_inputs.find( control1 )->second << "," << circ.lines_to_inputs.find( control2 )->second << "," << circ.lines_to_inputs.find( target )->second << ")" << std::endl;
        }
        gates.close();
    }
  }
  
  void create_toffoli( circuit& circ, const gate::line& control1, const gate::line& control2, const gate::line& control3, const gate::line& target )
  {
    std::ofstream gates;
    gates.open("gates.txt", std::ios_base::app);
    if (gates.is_open()){
        gates << "Toffoli (" << circ.lines_to_inputs.find( control1 )->second << "," << circ.lines_to_inputs.find( control2 )->second << "," << circ.lines_to_inputs.find( control3 )->second << "," << circ.lines_to_inputs.find( target )->second << ")" << std::endl;
        gates.close();
    }
  }

  gate& create_fredkin( gate& g, const gate::line_container& controls, const gate::line& target1, const gate::line& target2 )
  {
    std::for_each( controls.begin(), controls.end(),  boost::bind( &gate::add_control, &g, _1) );

    g.add_target( target1 );
    g.add_target( target2 );
    g.set_type( fredkin_tag() );

    return g;
  }

  gate& create_peres( gate& g, const gate::line& control, const gate::line& target1, const gate::line& target2 )
  {
    g.add_control( control );
    g.add_target( target1 );
    g.add_target( target2 );

    peres_tag tag;
    tag.swap_targets = target1 > target2;
    g.set_type( tag );

    return g;
  }

  void create_cnot( circuit& circ, const gate::line& control, const gate::line& target )
  {
    std::ofstream gates;
    gates.open("gates.txt", std::ios_base::app);
    if (gates.is_open()){
        std::string in1 = circ.lines_to_inputs.find( control )->second;
        std::string in2 = circ.lines_to_inputs.find( target )->second;
        int count1 = registerCount++;
        int count2 = registerCount++;
        if( LLVM_IR ) {
            gates << "  %" << count1 << " = load i16* %" << in1 << ", align 2\n"; 
            gates << "  %" << count2 << " = load i16* %" << in2 << ", align 2\n"; 
            gates << "  call void @llvm.CNOT( i16 %" << count1 << ", i16 %" << count2 << ")\n";

        }
        else
            gates << "CNOT (" << circ.lines_to_inputs.find( control )->second << "," << circ.lines_to_inputs.find( target )->second << ")" << std::endl; 
        gates.close();
    }
  }

  gate& create_v( gate& g, const gate::line& control, const gate::line& target )
  {
    g.add_control( control );
    g.add_target( target );
    g.set_type( v_tag() );

    return g;
  }

  gate& create_vplus( gate& g, const gate::line& control, const gate::line& target )
  {
    g.add_control( control );
    g.add_target( target );
    g.set_type( vplus_tag() );

    return g;
  }

  void create_not( circuit& circ, const gate::line& target )
  {
    std::ofstream gates;
    gates.open("gates.txt", std::ios_base::app);
    if (gates.is_open()){
        std::string in1 = circ.lines_to_inputs.find( target )->second;
        int count1 = registerCount++;
        if( LLVM_IR ) {
            gates << "  %" << count1 << " = load i16* %" << in1 << ", align 2\n"; 
            gates << "  call void @llvm.X( i16 %" << count1 << ")\n";
        }
        else 
            gates << "X (" << circ.lines_to_inputs.find( target )->second << ")" << std::endl;
        gates.close();
    }
  }

  gate& create_module( gate& g, const circuit& circ, const std::string& name, const gate::line_container& controls, const std::vector<unsigned>& targets )
  {
    typedef std::map<std::string, boost::shared_ptr<circuit> > map_t;
    const map_t& modules = circ.modules();
    map_t::const_iterator it = modules.find( name );
    assert( it != modules.end() );

    boost::for_each( controls, boost::bind( &gate::add_control, &g, _1 ) );
    boost::for_each( targets, boost::bind( &gate::add_target, &g, _1 ) );

    module_tag module;
    module.reference = it->second;
    module.name = name;

    // sort order
    std::vector<unsigned> targets_sorted( targets.begin(), targets.end() );
    boost::sort( targets_sorted );
    foreach ( unsigned index, targets )
    {
      module.target_sort_order += std::distance( boost::begin( targets_sorted ), boost::find( targets_sorted, index ) );
    }

    g.set_type( module );
    return g;
  }

  ////////////////////////////// append_ functions

  void toffoli( qint control1, qint control2, qint target)
  {
    int i;
    for(i = 0; i < control1.reg.size(); i++){
      create_toffoli( control1.circ, control1.reg[i]->id, control2.reg[i]->id, target.reg[i]->id );
    }
  }

  void toffoli( qint control1, qint control2, qint control3, qint target)
  {
    int i;
    for(i = 0; i < control1.reg.size(); i++){
      create_toffoli( control1.circ, control1.reg[i]->id, control2.reg[i]->id, control3.reg[i]->id, target.reg[i]->id );
    }
  }





  gate& append_fredkin( circuit& circ, const gate::line_container& controls, const gate::line& target1, const gate::line& target2 )
  {
    return create_fredkin( circ.append_gate(), controls, target1, target2 );
  }

  gate& append_peres( circuit& circ, const gate::line& control, const gate::line& target1, const gate::line& target2 )
  {
    return create_peres( circ.append_gate(), control, target1, target2 );
  }

  void cnot( qint control, qint target )
  {
    int i;
    for(i = 0; i < control.reg.size(); i++){
      create_cnot( control.circ, control.reg[i]->id, target.reg[i]->id );
    }
  }

  void append_cnot( circuit& circ, const gate::line& control, const gate::line& target )
  {
    create_cnot( circ, control, target );
  }

  gate& append_v( circuit& circ, const gate::line& control, const gate::line& target )
  {
    return create_v( circ.append_gate(), control, target );
  }

  gate& append_vplus( circuit& circ, const gate::line& control, const gate::line& target )
  {
    return create_vplus( circ.append_gate(), control, target );
  }

  void NOT( qint target )
  {
    int i;
    for(i = 0; i < target.reg.size(); i++){
      create_not( target.circ, target.reg[i]->id );
    }
  }

/*-------------- CTQG Functions -----------------*/

  void assign_value_of_int_to_a( qint a, int b, int size)
  {
    zero_to_garbage ancilla(size);

    int i = 0;
    while (i < size) 
    {
      cnot( a[i], ancilla[i] );
      cnot( ancilla[i], a[i] );
      if( b == 0 ){
        zero_to_zero anc;
        cnot( anc, ancilla[i] );
      }
      else {
        one_to_one anc;
        cnot( anc, ancilla[i] );
      }
      i++;
    }
  }

  void assign_value_of_b_to_a( qint a, qint b, int size )
  {
    zero_to_garbage ancilla(size);
    int i = 0;
    while ( i < size )
    {
      cnot( a[i], ancilla[i] );
      cnot( ancilla[i], a[i] );
      cnot( b[i], a[i] );
      i++;
    }
  }

  void assign_value_of_b_to_a( qint a, std::string binary_constant, int size )
  {
    zero_to_garbage ancilla(size);
    int num_zero = 0;
    int num_one = 0;
    int i = 0;
    for( std::string::iterator it = binary_constant.begin(); it != binary_constant.end(); ++it ) 
    {
        if (i > size )
        {
            std::cout << "Size Error" << std::endl;
            break;
        }           
        cnot ( a[i], ancilla[i] );
        cnot ( ancilla[i], a[i] );
        if (*it == '0') 
        {
            zero_to_zero anc;
            cnot ( anc, a[i] );
        }
        if (*it == '1')
        {
            one_to_one anc;
            cnot ( anc, a[i] );
        }
        i++;
    } 
  }


  void a_eq_a_plus_b( qint x, qint y, int size )
  {
  /*------ CTQG Adder ------
    int i = size - 1;
    
    while( i > 0 )
    {
      cnot ( x[i], y[i] );
      cnot ( x[i-1], x[i] );
      i--;
    }
    cnot ( x[0], y[0] );

    i = 1;

    while( i < size )
    {
      toffoli ( y[i-1], x[i-1], x[i] );
      i++;
    }
    toffoli ( y[size-2], x[size-2], y[size-1] );

    i = size-1;

    while ( i >= 2 )
    {
      cnot ( y[i-1], y[i] );
      toffoli ( y[i-2], x[i-2], y[i-1] ); 
      i--;
    }

    i = 1;
    
    cnot ( y[0], y[1] );

    while ( i < size )
    {
      cnot ( x[i-1], y[i-1] );
      cnot ( y[i-1], x[i-1] );
      cnot ( y[i-1], y[i] ); 
      i++;
    } 

    cnot ( x[i-1], y[i-1] );
    cnot ( y[i-1], x[i-1] );
  */



/*------- Improved Adder - With Ancilla -------- */
    zero_to_garbage zero(size);
    zero_to_garbage ancilla(size);
    int i = 0;
    for(i=0; i < size; i++){
        cnot(x[i], ancilla[i]);
        cnot(y[i], zero[i]);
        cnot(x[i], y[i]);
        toffoli(y[i], ancilla[i], x[i]);
        cnot(zero[i], ancilla[i]);
        cnot(zero[i], y[i]);
    }

//Classical bookkeeping change: y:=zero, x:=y registers
//Avoids adding additional gates to swap values
   for(i=0; i<size; i++){
        unsigned temp1 = y.reg[i]->id;
        unsigned temp2 = x.reg[i]->id;
        y.reg[i]->id = zero.reg[i]->id;
        x.reg[i]->id = ancilla.reg[i]->id;
        zero.reg[i]->id = temp1;
        ancilla.reg[i]->id = temp2;
    }
  }


  void a_eq_a_minus_b( qint x, qint y, int size)
  {

    int i = size-1;
    while ( i > 0 )
    {
      cnot ( y[i], x[i] ); 
      cnot ( x[i], y[i] );
      cnot ( y[i-1], y[i] );
      i--;
    } 
    cnot ( y[i], x[i] ); 
    cnot ( x[i], y[i] );
    cnot ( y[i], y[i+1] );

    i = 2; 

    while ( i <= size-1 )
    {
      toffoli ( y[i-2], x[i-2], y[i-1] ); 
      cnot ( y[i-1], y[i] );
      i++;
    }

    i = size-1;

    toffoli ( y[size-2], x[size-2], y[size-1] );
    while( i > 0 )
    {
      toffoli ( y[i-1], x[i-1], x[i] );
      i--;
    }
    

    i = 1;
    
    cnot ( x[0], y[0] );
    while( i <= size-1 )
    {
      cnot ( x[i-1], x[i] );
      cnot ( x[i], y[i] );
      i++;
    }

  }


  void a_eq_a_plus_b_times_c( qint a, qint b, qint c, int size)
  {
    int i;
    int j;
    
    for(j = 0; j < size - 1; j++){
        for(i=1; i < size-j; i++){
            toffoli ( b[j], a[size-i], c[size-i-j] );
            toffoli ( b[j], a[size-i-1], a[size-i] );
        }
        if( j == 0 ) toffoli( b[0], a[0], c[0] );
        else toffoli (b[j], a[j], c[0] );

        for(i=0; i <= size - 2 - j; i++){
            toffoli ( b[j], c[i], a[i+j], a[i+j+1] );
        } 
        toffoli( b[j], c[size - 2 - j], a[size - 2], c[size - 1 - j] );

        for(i = size - 2; i > 0 + j; i--){
            toffoli ( b[j], c[i - j], c[i + 1 - j] ); 
            toffoli ( b[j], c[i - 1 - j], a[i - 1], c[i - j] );
        }

        toffoli( b[j], c[0], c[1] );
        toffoli( b[j], a[0+j], c[0] );
        toffoli( b[j], c[0], a[0+j] );

        for(i = 1; i <= size - 1 - j; i++){
            toffoli( b[j], c[i - 1], c[i] );
            toffoli( b[j], a[i+j], c[i] );
            toffoli( b[j], c[i], a[i+j] );
        }
    }
    toffoli( b[size - 1], a[size - 1], c[0] );
    toffoli( b[size - 1], a[size - 1], c[0] );
    toffoli( b[size - 1], c[0], a[size - 1] );
  }



  void a_swap_b( qint a, qint b, int size)
  {
    while (size > 0)
    {
      cnot ( a[size-1], b[size-1] );
      cnot ( b[size-1], a[size-1] );
      cnot ( a[size-1], b[size-1] );
      size--;
    }
  }

/*------- Gate Creation Functions ------------*/

  void append_not( circuit& circ, const gate::line& target )
  {
    create_not( circ, target );
  }

  gate& append_module( circuit& circ, const std::string& module_name, const gate::line_container& controls, const std::vector<unsigned>& targets )
  {
    return create_module( circ.append_gate(), circ, module_name, controls, targets );
  }

  control_line_adder append_gate( circuit& circ, const boost::any& tag )
  {
    gate& g = circ.append_gate();
    g.set_type( tag );
    return control_line_adder( g );
  }

  control_line_adder append_toffoli( circuit& circ )
  {
    return append_gate( circ, toffoli_tag() );
  }

  control_line_adder append_fredkin( circuit& circ )
  {
    return append_gate( circ, fredkin_tag() );
  }

  control_line_adder append_v( circuit& circ )
  {
    return append_gate( circ, v_tag() );
  }

  control_line_adder append_vnot( circuit& circ )
  {
    return append_gate( circ, vplus_tag() );
  }

  ////////////////////////////// prepend_ functions

  gate& prepend_fredkin( circuit& circ, const gate::line_container& controls, const gate::line& target1, const gate::line& target2 )
  {
    return create_fredkin( circ.prepend_gate(), controls, target1, target2 );
  }

  gate& prepend_peres( circuit& circ, const gate::line& control, const gate::line& target1, const gate::line& target2 )
  {
    return create_peres( circ.prepend_gate(), control, target1, target2 );
  }

  void prepend_cnot( circuit& circ, const gate::line& control, const gate::line& target )
  {
    create_cnot( circ, control, target );
  }

  gate& prepend_v( circuit& circ, const gate::line& control, const gate::line& target )
  {
    return create_v( circ.prepend_gate(), control, target );
  }

  gate& prepend_vplus( circuit& circ, const gate::line& control, const gate::line& target )
  {
    return create_vplus( circ.prepend_gate(), control, target );
  }

  void prepend_not( circuit& circ, const gate::line& target )
  {
    create_not( circ, target );
  }

  gate& prepend_module( circuit& circ, const std::string& module_name, const gate::line_container& controls, const std::vector<unsigned>& targets )
  {
    return create_module( circ.prepend_gate(), circ, module_name, controls, targets );
  }

  control_line_adder prepend_gate( circuit& circ, const boost::any& tag )
  {
    gate& g = circ.prepend_gate();
    g.set_type( tag );
    return control_line_adder( g );
  }

  control_line_adder prepend_toffoli( circuit& circ )
  {
    return prepend_gate( circ, toffoli_tag() );
  }

  control_line_adder prepend_fredkin( circuit& circ )
  {
    return prepend_gate( circ, fredkin_tag() );
  }

  control_line_adder prepend_v( circuit& circ )
  {
    return prepend_gate( circ, v_tag() );
  }

  control_line_adder prepend_vnot( circuit& circ )
  {
    return prepend_gate( circ, vplus_tag() );
  }

  ////////////////////////////// insert_ functions

  gate& insert_fredkin( circuit& circ, unsigned n, const gate::line_container& controls, const gate::line& target1, const gate::line& target2 )
  {
    return create_fredkin( circ.insert_gate( n ), controls, target1, target2 );
  }

  gate& insert_peres( circuit& circ, unsigned n, const gate::line& control, const gate::line& target1, const gate::line& target2 )
  {
    return create_peres( circ.insert_gate( n ), control, target1, target2 );
  }

  void insert_cnot( circuit& circ, unsigned n, const gate::line& control, const gate::line& target )
  {
    create_cnot( circ, control, target );
  }

  gate& insert_v( circuit& circ, unsigned n, const gate::line& control, const gate::line& target )
  {
    return create_v( circ.insert_gate( n ), control, target );
  }

  gate& insert_vplus( circuit& circ, unsigned n, const gate::line& control, const gate::line& target )
  {
    return create_vplus( circ.insert_gate( n ), control, target );
  }

  void insert_not( circuit& circ, unsigned n, const gate::line& target )
  {
    create_not( circ, target );
  }

  gate& insert_module( circuit& circ, unsigned n, const std::string& module_name, const gate::line_container& controls, const std::vector<unsigned>& targets )
  {
    return create_module( circ.insert_gate( n ), circ, module_name, controls, targets );
  }

  control_line_adder insert_gate( circuit& circ, unsigned n, const boost::any& tag )
  {
    gate& g = circ.insert_gate( n );
    g.set_type( tag );
    return control_line_adder( g );
  }

  control_line_adder insert_toffoli( circuit& circ, unsigned n )
  {
    return insert_gate( circ, n, toffoli_tag() );
  }

  control_line_adder insert_fredkin( circuit& circ, unsigned n )
  {
    return insert_gate( circ, n, fredkin_tag() );
  }

  control_line_adder insert_v( circuit& circ, unsigned n )
  {
    return insert_gate( circ, n, v_tag() );
  }

  control_line_adder insert_vnot( circuit& circ, unsigned n )
  {
    return insert_gate( circ, n, vplus_tag() );
  }
}
