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

#include "write_blif.hpp"

#include <iterator>

#include <boost/algorithm/string/join.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/regex.hpp>
#include <boost/tuple/tuple.hpp>

#include "../circuit.hpp"
#include "../target_tags.hpp"
#include "../functions/target_lines.hpp"

using namespace boost::assign;

namespace revkit
{

  struct is_primary
  {
    template<typename T>
    bool operator()( const T& tuple ) const
    {
      return !boost::get<1>( tuple );
    }
  };

  template<int N, typename T>
  inline
  const typename boost::tuples::element<N,T>::type& getN( const T& t )
  {
    return boost::tuples::get<N>( t );
  }

  struct signal_name
  {
    signal_name( unsigned& tmp_signal, const std::string& tmp_signal_name, bool keep_constant_names, const std::vector<std::string>& inputs )
      : tmp_signal( tmp_signal ),
        tmp_signal_name( tmp_signal_name ),
        keep_constant_names( keep_constant_names ),
        inputs( inputs ) {}

    template<typename T>
    std::string operator()( const T& tuple ) const
    {
      if ( !boost::get<1>( tuple ) || ( keep_constant_names && boost::count( inputs, boost::get<0>( tuple ) ) <= 1u ) )
      {
        return boost::get<0>( tuple );
      }
      else
      {
        return boost::str( boost::format( "%s%d" ) % tmp_signal_name % tmp_signal++ );
      }
    }

  private:
    unsigned& tmp_signal;
    const std::string& tmp_signal_name;
    bool keep_constant_names;
    const std::vector<std::string>& inputs;
  };

  template<typename T>
  struct random_access
  {
    typedef typename T::value_type result_type;

    explicit random_access( const T& container ) : container( container ) {}

    const typename T::value_type& operator()( typename T::size_type n ) const
    {
      return container.at( n );
    }

  private:
    const T& container;
  };

  template<typename T>
  random_access<T> make_random_access( const T& container )
  {
    return random_access<T>( container );
  }

  struct to_blif
  {
    typedef std::string result_type;

    std::string operator()( const boost::optional<bool>& vt ) const
    {
      return vt ? ( *vt ? "1" : "0" ) : "-";
    }
  };

  struct to_blif_mv
  {
    typedef std::string result_type;

    std::string operator()( const boost::optional<bool>& vt ) const
    {
      return ( vt ? ( *vt ? "1" : "0" ) : "-" ) + std::string( " " );
    }
  };

  void rephrase_signal_names( std::vector<std::string>& signals, const bus_collection& bus, const std::string& prefix = std::string() )
  {
    for ( unsigned i = 0u; i < signals.size(); ++i )
    {
      std::string signalname = bus.has_bus( i ) ? boost::str( boost::format( "%s<%d>" ) % bus.find_bus( i ) % bus.signal_index( i ) ) : signals.at( i );
      signalname = boost::regex_replace( signalname, boost::regex( "\\[(\\d+)\\]" ), "<*\\1*>", boost::match_default | boost::format_sed );
      if ( bus.has_bus( i ) )
      {
        signalname = prefix + signalname;
      }
      signals.at( i ) = signalname;
    }
  }

  write_blif_settings::write_blif_settings()
    : tmp_signal_name( "tmp" ),
      blif_mv( false ),
      state_prefix( "out_" ),
      keep_constant_names( false )
  {
  }

  void write_blif_settings::operator()( const gate& g, truth_table_map& map ) const
  {
    unsigned num_controls = std::distance( g.begin_controls(), g.end_controls() );

    if ( is_toffoli( g ) )
    {
      std::map<std::vector<boost::optional<bool> >, bool> cubes;
      for ( unsigned j = 0; j < num_controls; ++j )
      {
        std::vector<boost::optional<bool> > cube( num_controls + 1u );
        cube.at( 0u ) = true;
        cube.at( 1u + j ) = false;

        cubes[cube] = true;
      }

      std::vector<boost::optional<bool> > cube( num_controls + 1u, true );
      cube.at( 0u ) = false;

      cubes[cube] = true;

      map[*g.begin_targets()] = cubes;
    }
    else if ( is_fredkin( g ) )
    {
      std::vector<std::string> not_equal;
      not_equal += "01","10";

      std::map<std::vector<boost::optional<bool> >, bool> cubes1, cubes2;

      for ( unsigned i = 0u; i < 2u; ++i )
      {
        for ( unsigned j = 0u; j < num_controls; ++j )
        {
          std::vector<boost::optional<bool> > cube( num_controls + 2u );
          cube.at( 0u ) = i != 0u;
          cube.at( 1u ) = i == 0u;
          cube.at( 2u + j ) = false;
          cubes1[cube] = i != 0u;
          cubes2[cube] = i == 0u;
        }

        std::vector<boost::optional<bool> > cube( num_controls + 2u, true );
        cube.at( 0u ) = i != 0u;
        cube.at( 1u ) = i == 0u;
        cubes1[cube] = i == 0u;
        cubes2[cube] = i != 0u;
      }

      std::vector<boost::optional<bool> > cube( num_controls + 2u );
      cube.at( 0u ) = cube.at( 1u ) = true;
      cubes1[cube] = true;
      cubes2[cube] = true;

      std::vector<unsigned> targets( 2u );
      target_lines( g, targets.begin() );
      map[targets.at( 0u )] = cubes1;
      map[targets.at( 1u )] = cubes2;
    }
    else if ( is_peres( g ) )
    {
      // peres has one control and two targets
      std::map<std::vector<boost::optional<bool> >, bool> cubes1, cubes2;

      std::vector<boost::optional<bool> > cube;
      cube += false,false,true;
      cubes2[cube] = true;

      cube.clear();
      cube += false,true,false;
      cubes2[cube] = true;

      cube.clear();
      cube += false,true,true;
      cubes1[cube] = true;

      cube.clear();
      cube += true,false,false;
      cubes1[cube] = true;

      cube.clear();
      cube += true,false,true;
      cubes1[cube] = cubes2[cube] = true;

      cube.clear();
      cube += true,true,false;
      cubes1[cube] = cubes2[cube] = true;

      std::vector<unsigned> targets( 2u );
      target_lines( g, targets.begin() );
      map[targets.at( 0u )] = cubes1;
      map[targets.at( 1u )] = cubes2;
    }
  }

  void write_blif( const circuit& circ, std::ostream& os, const write_blif_settings& settings )
  {
    std::vector<std::string> signals( circ.lines() );
    unsigned tmp_signal = 0;

    /* model name */
    std::string model_name = circ.circuit_name();
    if ( !model_name.size() )
    {
      model_name = "circuit";
    }

    os << ".model " << model_name << std::endl;

    /* override inputs and outputs with possible bus names */
    std::vector<std::string> _inputs( circ.inputs().begin(), circ.inputs().end() );
    std::vector<std::string> _outputs( circ.outputs().begin(), circ.outputs().end() );
    rephrase_signal_names( _inputs, circ.inputbuses() );
    rephrase_signal_names( _inputs, circ.statesignals() );
    rephrase_signal_names( _outputs, circ.outputbuses() );
    rephrase_signal_names( _outputs, circ.statesignals(), settings.state_prefix );

    /* zip inputs and outputs */
    std::vector<boost::tuple<std::string, constant> > inputs( circ.lines() );
    std::copy( boost::make_zip_iterator( boost::make_tuple( _inputs.begin(), circ.constants().begin() ) ),
               boost::make_zip_iterator( boost::make_tuple( _inputs.end(), circ.constants().end() ) ),
               inputs.begin() );

    typedef boost::tuple<std::string, bool> out_tuple;
    std::vector<out_tuple> outputs( circ.lines() );
    std::copy( boost::make_zip_iterator( boost::make_tuple( _outputs.begin(), circ.garbage().begin() ) ),
               boost::make_zip_iterator( boost::make_tuple( _outputs.end(), circ.garbage().end() ) ),
               outputs.begin() );

    /* inputs */
    os << ".inputs ";
    std::transform( boost::make_filter_iterator<is_primary>( inputs.begin(), inputs.end() ),
                    boost::make_filter_iterator<is_primary>( inputs.end(), inputs.end() ),
                    std::ostream_iterator<std::string>( os, " " ),
                    getN<0, boost::tuple<std::string, constant> > );
    os << std::endl;

    /* outputs */
    os << ".outputs ";
    std::transform( boost::make_filter_iterator<is_primary>( outputs.begin(), outputs.end() ),
                    boost::make_filter_iterator<is_primary>( outputs.end(), outputs.end() ),
                    std::ostream_iterator<std::string>( os, " " ),
                    getN<0, out_tuple> );
    os << std::endl;

    std::transform( inputs.begin(), inputs.end(), signals.begin(), signal_name( tmp_signal, settings.tmp_signal_name, settings.keep_constant_names, circ.inputs() ) );

    /* constants */
    unsigned i = 0u;
    foreach ( const constant& c, circ.constants() )
    {
      if ( c )
      {
        os << ".names " << signals.at( i ) << std::endl
           << ".def 0" << std::endl
           << "- " << ( *c ? "1" : "0" ) << std::endl;
      }
      ++i;
    }

    foreach ( const gate& g, circ )
    {
      using boost::adaptors::transformed;
      write_blif_settings::truth_table_map ttm;

      // calculate truth table map
      settings( g, ttm );

      // input signature
      std::string input_signature =
        boost::join( boost::make_iterator_range( g.begin_targets(), g.end_targets() )
                     | transformed( make_random_access( signals ) ), " " ) + " " +
        boost::join( boost::make_iterator_range( g.begin_controls(), g.end_controls() )
                     | transformed( make_random_access( signals ) ), " " );

      foreach ( unsigned target, boost::make_iterator_range( g.begin_targets(), g.end_targets() ) )
      {
        // update name
        signals.at( target ) = boost::str( boost::format( "%s%d" ) % settings.tmp_signal_name % tmp_signal );

        // write signature
        os << boost::format( ".names %s %s%d" ) % input_signature % settings.tmp_signal_name % ( tmp_signal ) << std::endl;
        ++tmp_signal;
        os << ".def 0" << std::endl;

        // write truth table
        foreach ( const write_blif_settings::truth_table_map::mapped_type::value_type& pair, ttm[target] )
        {
          if ( !pair.second ) continue; // omit 0 outputs
          boost::function<std::string(const boost::optional<bool>&)> transformer;
          if ( settings.blif_mv )
          {
            transformer = to_blif_mv();
          }
          else
          {
            transformer = to_blif();
          }
          boost::copy( pair.first | transformed( transformer ), std::ostream_iterator<std::string>( os ) );
          if ( !settings.blif_mv )
          {
            os << ' ';
          }
          os << '1' << std::endl;
        }
      }
    }

    foreach ( const out_tuple& t, outputs )
    {
      if ( !boost::get<1>( t ) )
      {
        os << ".names "
           << signals.at( std::find( _outputs.begin(), _outputs.end(), boost::get<0>( t ) ) - _outputs.begin() )
           << " "
           << boost::get<0>( t ) << std::endl
           << ".def 0" << std::endl
           << "1 1" << std::endl;
      }
    }

    os << ".end" << std::endl;
  }

}
