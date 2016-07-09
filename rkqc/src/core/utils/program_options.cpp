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

#include "program_options.hpp"

#include "costs.hpp"

namespace revkit
{

  class program_options::priv
  {
  public:
    priv()
      : costs( 0 ),
        has_in_realization( false ),
        has_in_specification( false ),
        has_out_realization( false ),
        has_costs( false ),
        parsed( false )
    {}
  
    std::string in_realization;
    std::string in_specification;
    std::string out_realization;
    unsigned costs;

    bool has_in_realization;
    bool has_in_specification;
    bool has_out_realization;
    bool has_costs;

    bool parsed;

    boost::program_options::variables_map vm;
    boost::program_options::positional_options_description pod;
  };

  program_options::program_options( unsigned line_length )
    : boost::program_options::options_description( line_length ),
      d( new priv() )
  {
    init();
  }

  program_options::program_options( const std::string& caption, unsigned line_length )
    : boost::program_options::options_description( caption, line_length ),
      d( new priv() )
  {
    init();
  }
  
  program_options::~program_options()
  {
    delete d;
  }

  bool program_options::good() const
  {
    return ( !d->vm.count( "help" ) && ( !d->has_in_realization || d->vm.count( "filename" ) ) && ( !d->has_in_specification || d->vm.count( "filename" ) ) );
  }

  void program_options::parse( int argc, char ** argv )
  {
    d->pod.add( "filename", 1 );

    boost::program_options::store( boost::program_options::command_line_parser( argc, argv ).options( *this ).positional( d->pod ).run(), d->vm );
    boost::program_options::notify( d->vm );
    d->parsed = true;
  }

  bool program_options::is_set( const std::string& option ) const
  {
    if ( !d->parsed )
    {
      return false;
    }
    return d->vm.count( option ) == 1;
  }

  program_options& program_options::add_read_realization_option()
  {
    assert( !( d->has_in_realization || d->has_in_specification ) );
    add_options()( "filename", boost::program_options::value<std::string>( &d->in_realization ), "circuit realization in RevLib *.real format" );
    d->has_in_realization = true;

    return *this;
  }

  program_options& program_options::add_read_specification_option()
  {
    assert( !( d->has_in_realization || d->has_in_specification ) );
    add_options()( "filename", boost::program_options::value<std::string>( &d->in_specification ), "circuit specification in RevLib *.spec format" );
    d->has_in_specification = true;

    return *this;
  }

  program_options& program_options::add_write_realization_option()
  {
    add_options()( "realname", boost::program_options::value<std::string>( &d->out_realization ), "output circuit realization in RevLib *.real format" );
    d->has_out_realization = true;

    return *this;
  }

  program_options& program_options::add_costs_option()
  {
    add_options()( "costs", boost::program_options::value<unsigned>( &d->costs )->default_value( 0 ), "0: Gate Costs\n1: Line Costs\n2: Quantum Costs\n3: Transistor Costs" );
    d->has_costs = true;

    return *this;
  }

  const std::string& program_options::read_realization_filename() const
  {
    return d->in_realization;
  }

  const std::string& program_options::read_specification_filename() const
  {
    return d->in_specification;
  }

  const std::string& program_options::write_realization_filename() const
  {
    return d->out_realization;
  }

  cost_function program_options::costs() const
  {
    assert( d->has_costs );

    switch ( d->costs )
    {
    case 0:
      return costs_by_circuit_func( gate_costs() );
    case 1:
      return costs_by_circuit_func( line_costs() );
    case 2:
      return costs_by_gate_func( quantum_costs() );
    case 3:
      return costs_by_gate_func( transistor_costs() );
    default:
      assert( false );
      return cost_function();
    }
  }

  bool program_options::is_write_realization_filename_set() const
  {
    if ( !d->parsed || !d->has_out_realization )
    {
      return false;
    }

    return d->vm.count( "realname" );
  }

  void program_options::init()
  {
    add_options()( "help", "produce help message" );
  }

  const boost::program_options::variables_map& program_options::variables_map() const
  {
    return d->vm;
  }
}
