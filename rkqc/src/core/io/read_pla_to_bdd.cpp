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

#include "read_pla_to_bdd.hpp"

#include <fstream>

#include <boost/assign/std/vector.hpp>
#include <boost/bind.hpp>
#include <boost/config/warning_disable.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/range/irange.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/tuple/tuple.hpp>

#define foreach BOOST_FOREACH

using namespace boost::assign;

namespace revkit
{

  namespace qi = boost::spirit::qi;
  namespace ascii = boost::spirit::ascii;

  typedef boost::fusion::vector<std::string, unsigned> ast_num_vars;
  typedef boost::fusion::vector<std::string, std::vector<std::string> > ast_labels;
  typedef std::string ast_type;
  typedef boost::fusion::vector<std::string, std::string> ast_cube;
  typedef boost::fusion::vector<std::vector<boost::variant<ast_num_vars, ast_labels, ast_type> >, std::vector<ast_cube> > ast_pla;

  template<typename Iterator>
  struct pla_skip_parser : qi::grammar<Iterator>
  {
    explicit pla_skip_parser() : pla_skip_parser::base_type( base_rule )
    {
      using qi::char_;
      using qi::eol;
      using ascii::space;

      base_rule = ( space - qi::eol ) | single_line_comment_rule;

      single_line_comment_rule = "#" >> *( char_ - eol ) >> +eol;
    }

    qi::rule<Iterator> base_rule;
    qi::rule<Iterator> single_line_comment_rule;
  };

  template<typename Iterator, typename SpaceT>
  struct pla_parser : qi::grammar<Iterator, ast_pla(), SpaceT>
  {
    explicit pla_parser() : pla_parser::base_type( main_rule )
    {
      using qi::char_;
      using qi::eol;
      using qi::uint_;
      using qi::lexeme;
      using qi::string;
      using ascii::alnum;
      using ascii::space;

      identifier %= lexeme[+( alnum | '_' )];
      num_vars_rule %= ( string( ".i" ) | string( ".o" ) | string( ".p" ) ) >> uint_ >> +eol;
      labels_rule %= ( string( ".ilb" ) | string( ".ob" ) ) >> +identifier >> +eol;
      type_rule %= ".type" >> ( string( "pla" ) | string( "esop" ) ) >> +eol;
      cube_rule %= lexeme[+char_( "01-" )] >> lexeme[+char_( "01~-" )] >> +eol;

      main_rule %= +( num_vars_rule | labels_rule | type_rule ) >> +cube_rule >> ".e" >> +eol;
    }

    qi::rule<Iterator, std::string(), SpaceT> identifier;

    qi::rule<Iterator, ast_num_vars(), SpaceT> num_vars_rule;
    qi::rule<Iterator, ast_labels(), SpaceT> labels_rule;
    qi::rule<Iterator, ast_type(), SpaceT> type_rule;
    qi::rule<Iterator, ast_cube(), SpaceT> cube_rule;
    qi::rule<Iterator, ast_pla(), SpaceT> main_rule;
  };

  struct pla_t
  {
    boost::optional<unsigned> num_inputs;
    boost::optional<unsigned> num_outputs;
    std::vector<std::string> input_labels;
    std::vector<std::string> output_labels;
    std::string type;
    std::vector<boost::tuples::tuple<std::string, std::string> > cubes;
  };

  struct ast_pla_visitor : public boost::static_visitor<>
  {
    explicit ast_pla_visitor( pla_t& p ) : p( p ) {}

    void operator()( const ast_num_vars& t ) const
    {
      if ( boost::fusion::at_c<0>( t ) == ".i" )
      {
        p.num_inputs = boost::fusion::at_c<1>( t );
      }

      else if ( boost::fusion::at_c<0>( t ) == ".o" )
      {
        p.num_outputs = boost::fusion::at_c<1>( t );
      }
    }

    void operator()( const ast_labels& t ) const
    {
      if ( boost::fusion::at_c<0>( t ) == ".ilb" )
      {
        p.input_labels = boost::fusion::at_c<1>( t );
      }

      else if ( boost::fusion::at_c<0>( t ) == ".ob" )
      {
        p.output_labels = boost::fusion::at_c<1>( t );
      }
    }

    void operator()( const ast_type& t ) const
    {
      p.type = t;
    }
  private:
    pla_t& p;
  };

  bool semantic_parse( const ast_pla& pla, pla_t& p )
  {
    ast_pla_visitor visitor( p );
    boost::for_each( boost::fusion::at_c<0>( pla ), boost::apply_visitor( visitor ) );

    typedef boost::fusion::result_of::value_at<ast_pla, boost::mpl::int_<1> >::type::value_type fusion_t;
    foreach ( const fusion_t& f, boost::fusion::at_c<1>( pla ) )
    {
      p.cubes += boost::tuples::make_tuple( boost::fusion::at_c<0>( f ), boost::fusion::at_c<1>( f ) );
    }

    // TODO transform into error messages
    assert( p.num_inputs );
    assert( p.num_outputs );

    // Auto Generate input and output labels if they do not exist
    if ( p.input_labels.empty() )
    {
      foreach ( unsigned i, boost::irange( 0u, *p.num_inputs ) )
      {
        p.input_labels += boost::str( boost::format( "i%d" ) % i );
      }
    }

    if ( p.output_labels.empty() )
    {
      foreach ( unsigned i, boost::irange( 0u, *p.num_outputs ) )
      {
        p.output_labels += boost::str( boost::format( "o%d" ) % i );
      }
    }

    // TODO transform into error messages
    assert( *p.num_inputs == p.input_labels.size() );
    assert( *p.num_outputs == p.output_labels.size() );

    return true;
  }

  template<typename Iterator>
  bool parse( ast_pla& pla, Iterator first, Iterator last )
  {
    pla_parser<Iterator, pla_skip_parser<Iterator> > parser;
    pla_skip_parser<Iterator> skip_parser;

    bool r = qi::phrase_parse( first, last,
                               parser,
                               skip_parser,
                               pla );

    if ( !r || first != last )
    {
      std::cerr << " error at: " << std::string( first, last ) << std::endl;
      return false;
    }

    return true;
  }

  bool parse( pla_t& pla, const std::string& filename )
  {
    ast_pla p;

    std::string content, line;

    std::ifstream is;
    is.open( filename.c_str(), std::ios::in );

    while ( getline( is, line ) )
    {
      content += line + '\n';
    }

    if ( !parse( p, content.begin(), content.end() ) )
    {
      return false;
    }

    if ( !semantic_parse( p, pla ) )
    {
      return false;
    }

    return true;
  }

  bool read_pla_to_bdd( BDDTable& bdd, const std::string& filename )
  {
    using boost::adaptors::map_values;

    pla_t pla;

    if ( !parse( pla, filename ) )
    {
      return false;
    }

    // Inputs
    boost::transform( pla.input_labels,
                      std::back_inserter( bdd.inputs ),
                      boost::bind( std::make_pair<std::string, DdNode*>, _1, (DdNode*)0 ) );
    boost::generate( bdd.inputs | map_values, boost::bind( Cudd_bddNewVar, bdd.cudd ) );

    // Outputs
    boost::transform( pla.output_labels,
                      std::back_inserter( bdd.outputs ),
                      boost::bind( std::make_pair<std::string, DdNode*>, _1, Cudd_ReadLogicZero( bdd.cudd ) ) );
    boost::for_each( bdd.outputs | map_values, Cudd_Ref );


    // Iterate through cubes
    typedef boost::tuples::tuple<std::string, std::string> tuple_t;
    foreach ( const tuple_t& cube, pla.cubes )
    {
      const std::string& in = boost::get<0>( cube );
      const std::string& out = boost::get<1>( cube );

      DdNode *tmp, *var;
      DdNode* prod = Cudd_ReadOne( bdd.cudd );
      Cudd_Ref( prod );

      for ( unsigned i = 0u; i < *pla.num_inputs; ++i )
      {
        if ( in[i] == '-' ) continue;

        var = bdd.inputs[i].second;
        Cudd_Ref( var );
        if ( in[i] == '0' ) var = Cudd_Not( var );
        tmp = Cudd_bddAnd( bdd.cudd, prod, var );
        Cudd_Ref( tmp );
        Cudd_RecursiveDeref( bdd.cudd, prod );
        Cudd_RecursiveDeref( bdd.cudd, var );
        prod = tmp;
      }

      for ( unsigned i = 0u; i < *pla.num_outputs; ++i )
      {
        if ( out[i] == '0' || out[i] == '~' ) continue;

        tmp = Cudd_bddOr( bdd.cudd, bdd.outputs[i].second, prod );
        Cudd_Ref( tmp );
        Cudd_RecursiveDeref( bdd.cudd, bdd.outputs[i].second );
        bdd.outputs[i].second = tmp;
      }

      Cudd_RecursiveDeref( bdd.cudd, prod );
    }

    return true;
  }

  BDDTable::BDDTable()
  {
    cudd = Cudd_Init( 0, 0, CUDD_UNIQUE_SLOTS, CUDD_CACHE_SLOTS, 0 );
  }

  BDDTable::~BDDTable()
  {
    using boost::adaptors::map_values;

    boost::for_each( outputs | map_values, boost::bind( Cudd_RecursiveDeref, cudd, _1 ) );
    Cudd_Quit( cudd );
  }

}
