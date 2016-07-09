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

#include "create_image.hpp"

#include <fstream>
#include <sstream>

#include <boost/assign/std/vector.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/lambda/lambda.hpp>

#include "../target_tags.hpp"
#include "../functions/control_lines.hpp"
#include "../functions/target_lines.hpp"

#define foreach BOOST_FOREACH

using namespace boost::assign;

namespace revkit
{

  //// class: create_image_settings ////
  void create_image_settings::draw_before( std::ostream& os ) const
  {
    os << draw_before_text;
  }

  void create_image_settings::draw_in_between( std::ostream& os ) const
  {
    os << draw_in_between_text;
  }

  void create_image_settings::draw_after( std::ostream& os ) const
  {
    os << draw_after_text;
  }

  //// class: create_pstricks_settings ////

  create_pstricks_settings::create_pstricks_settings()
    : math_emph( true )
  {
    elem_width = 0.5;
    elem_height = 0.5;
    line_width = 0.01;
    control_radius = 0.1;
    target_radius = 0.2;
  }

  void create_pstricks_settings::draw_begin( std::ostream& os ) const
  {
    os << "\\begin{pspicture}(" << width << "," << height << ")" << std::endl;
  }

  void create_pstricks_settings::draw_line( std::ostream& os, float x1, float x2, float y ) const
  {
    os << boost::format( "\\psline[linewidth=%f](%f,%f)(%f,%f)" ) % line_width % x1 % y % x2 % y << std::endl;
  }

  void create_pstricks_settings::draw_input( std::ostream& os, float x, float y, const std::string& text, bool is_constant ) const
  {
    std::string input = text;
    if ( math_emph )
    {
      input = "$" + input + "$";
    }
    os << boost::format( "\\rput[r](%f,%f){%s%s}" ) % ( x - 0.1 ) % y % ( is_constant ? "\\color{red}" : "" ) % input << std::endl;
  }

  void create_pstricks_settings::draw_output( std::ostream& os, float x, float y, const std::string& text, bool is_garbage ) const
  {
    std::string output = text;
    if ( math_emph )
    {
      output = "$" + output + "$";
    }
    os << boost::format( "\\rput[l](%f,%f){%s}" ) % ( x + 0.1 ) % y % output << std::endl;
  }

  void create_pstricks_settings::draw_control( std::ostream& os, float x, float y ) const
  {
    os << boost::format( "\\pscircle*(%f,%f){%f}" ) % x % y % control_radius << std::endl;
  }

  void create_pstricks_settings::draw_targets( std::ostream& os, float x, const std::vector<float>& ys, const boost::any& target_tag ) const
  {
    if ( is_type<toffoli_tag>( target_tag ) )
    {
      os << boost::format( "\\pscircle[linewidth=%1%](%2%,%3%){%4%}\\psline[linewidth=%1%](%2%,%5%)(%2%,%6%)" ) % line_width % x % ys.at( 0 ) % target_radius % ( ys.at( 0 ) - target_radius ) % ( ys.at( 0 ) + target_radius );
    }
    else if ( is_type<fredkin_tag>( target_tag ) )
    {
      foreach ( const float& y, ys )
      {
        os << boost::format( "\\psline[linewidth=%1%](%2%,%3%)(%4%,%5%)\\psline[linewidth=%1%](%2%,%5%)(%4%,%3%)" )
          % line_width % ( x - control_radius ) % ( y - control_radius ) % ( x + control_radius ) % ( y + control_radius )
           << std::endl;
      }
    }
    else if ( is_type<v_tag>( target_tag ) )
    {
      os << boost::format( "\\psframe[linewidth=%1%,fillstyle=solid,fillcolor=white](%2%,%3%)(%4%,%5%) \\rput(%6%,%7%){\\sf V}" )
        % line_width % ( x - target_radius ) % ( ys.at( 0 ) - target_radius ) % ( x + target_radius ) % ( ys.at( 0 ) + target_radius ) % x % ys.at( 0 )
         << std::endl;
    }
    else if ( is_type<vplus_tag>( target_tag ) )
    {
      os << boost::format( "\\psframe[linewidth=%1%,fillstyle=solid,fillcolor=white](%2%,%3%)(%4%,%5%) \\rput(%6%,%7%){\\({\\sf V}^\\dagger\\)}" )
        % line_width % ( x - target_radius ) % ( ys.at( 0 ) - target_radius ) % ( x + target_radius ) % ( ys.at( 0 ) + target_radius ) % x % ys.at( 0 )
         << std::endl;
    }
  }

  void create_pstricks_settings::draw_peres_frame( std::ostream& os, float x1, float y1, float x2, float y2 ) const
  {
    os << boost::format( "\\psframe[linewidth=%1%,linestyle=dashed](%2%,%3%)(%4%,%5%)" ) % line_width % x1 % y1 % x2 % y2 << std::endl;
  }

  void create_pstricks_settings::draw_gate_line( std::ostream& os, float x, float y1, float y2 ) const
  {
    os << boost::format( "\\psline[linewidth=%f](%f,%f)(%f,%f)" ) % line_width % x % y1 % x % y2 << std::endl;
  }

  void create_pstricks_settings::draw_end( std::ostream& os ) const
  {
    os << "\\end{pspicture}" << std::endl;
  }

  //// class: create_tikz_settings ////

  create_tikz_settings::create_tikz_settings()
    : math_emph( true )
  {
    elem_width = 0.5;
    elem_height = 0.5;
    line_width = 0.3;
    control_radius = 0.1;
    target_radius = 0.2;
  }

  void create_tikz_settings::draw_begin( std::ostream& os ) const
  {
    os << "\\begin{tikzpicture}" << std::endl;
  }

  void create_tikz_settings::draw_line( std::ostream& os, float x1, float x2, float y ) const
  {
    os << boost::format( "\\draw[line width=%f] (%f,%f) -- (%f,%f);" ) % line_width % x1 % y % x2 % y << std::endl;
  }

  void create_tikz_settings::draw_input( std::ostream& os, float x, float y, const std::string& text, bool is_constant ) const
  {
    std::string input = text;
    if ( math_emph )
    {
      input = "$" + input + "$";
    }
    os << boost::format( "\\draw (%f,%f) node [left] {%s%s};" ) % ( x - 0.1 ) % y % ( is_constant ? "\\color{red}" : "" ) % input << std::endl;
  }

  void create_tikz_settings::draw_output( std::ostream& os, float x, float y, const std::string& text, bool is_garbage ) const
  {
    std::string output = text;
    if ( math_emph )
    {
      output = "$" + output + "$";
    }
    os << boost::format( "\\draw (%f,%f) node [right] {%s};" ) % ( x + 0.1 ) % y % output << std::endl;
  }

  void create_tikz_settings::draw_control( std::ostream& os, float x, float y ) const
  {
    os << boost::format( "\\draw[fill] (%f,%f) circle (%f);" ) % x % y % control_radius << std::endl;
  }

  void create_tikz_settings::draw_targets( std::ostream& os, float x, const std::vector<float>& ys, const boost::any& target_tag ) const
  {
    if ( is_type<toffoli_tag>( target_tag ) )
    {
      os << boost::format( "\\draw[line width=%1%] (%2%,%3%) circle (%4%) (%2%,%5%) -- (%2%,%6%);" ) % line_width % x % ys.at( 0 ) % target_radius % ( ys.at( 0 ) - target_radius ) % ( ys.at( 0 ) + target_radius ) << std::endl;
    }
    else if ( is_type<fredkin_tag>( target_tag ) )
    {
      foreach ( const float& y, ys )
      {
        os << boost::format( "\\draw[line width=%1%] (%2%,%3%) -- ++(%5%,%5%) (%2%,%4%) -- ++(%5%,-%5%);" )
          % line_width % ( x - control_radius ) % ( y - control_radius ) % ( y + control_radius ) % ( 2 * control_radius )
           << std::endl;
      }
    }
    else if ( is_type<v_tag>( target_tag ) )
    {
      os << boost::format( "\\draw[line width=%1%,fill=white] (%2%,%3%) rectangle ++(%4%,%4%); \\node at (%5%,%6%) {\\sf V};" ) % line_width % ( x - target_radius ) % ( ys.at( 0 ) - target_radius ) % ( 2 * target_radius ) % x % ys.at( 0 ) << std::endl;
    }
    else if ( is_type<vplus_tag>( target_tag ) )
    {
      os << boost::format( "\\draw[line width=%1%,fill=white] (%2%,%3%) rectangle ++(%4%,%4%); \\node at (%5%,%6%) {\\({\\sf V}^\\dagger\\)};" ) % line_width % ( x - target_radius ) % ( ys.at( 0 ) - target_radius ) % ( 2 * target_radius ) % x % ys.at( 0 ) << std::endl;
    }
  }

  void create_tikz_settings::draw_peres_frame( std::ostream& os, float x1, float y1, float x2, float y2 ) const
  {
    os << boost::format( "\\draw[line width=%1%,dashed] (%2%,%3%) rectangle (%4%,%5%);" ) % line_width % x1 % y1 % x2 % y2 << std::endl;
  }

  void create_tikz_settings::draw_gate_line( std::ostream& os, float x, float y1, float y2 ) const
  {
    os << boost::format( "\\draw[line width=%f] (%f,%f) -- (%f,%f);" ) % line_width % x % y1 % x % y2 << std::endl;
  }

  void create_tikz_settings::draw_end( std::ostream& os ) const
  {
    os << "\\end{tikzpicture}" << std::endl;
  }

  //// function: create_image ////

  void create_image( std::ostream& os, const circuit& circ, create_image_settings& settings )
  {
    if ( circ.num_gates() == 0 || circ.lines() == 0 )
    {
      return;
    }

    unsigned peres_count = std::count_if( circ.begin(), circ.end(), boost::bind( &is_peres, _1 ) );

    settings.width = settings.elem_width * ( 2 + circ.num_gates() + peres_count );
    settings.height = settings.elem_height * circ.lines();

    settings.draw_begin( os );

    settings.draw_before( os );

    float x1 = settings.elem_width;
    float x2 = settings.width - settings.elem_width;

    float y = settings.elem_height / 2;

    for ( unsigned i = 0; i < circ.lines(); ++i )
    {
      settings.draw_line( os, x1, x2, settings.height - y );
      settings.draw_input( os, x1, settings.height - y, ( circ.inputs().size() > i ? circ.inputs().at( i ) : "" ), circ.constants().at( i ) );
      settings.draw_output( os, x2, settings.height - y, ( circ.outputs().size() > i ? circ.outputs().at( i ) : "" ), circ.garbage().at( i ) );

      y += settings.elem_height;
    }

    settings.draw_in_between( os );

    float x = settings.elem_width * 3 / 2;

    foreach ( const gate& g, circ )
    {
      std::stringstream sstr;

      if ( !is_peres( g ) )
      {
        float ymin = settings.height;
        float ymax = 0;

        std::vector<unsigned> controls;
        control_lines( g, std::back_inserter( controls ) );

        foreach ( unsigned& control_index, controls )
        {
          float y = settings.height - ( control_index + 0.5 ) * settings.elem_height;
          ymin = std::min( ymin, y );
          ymax = std::max( ymax, y );
          settings.draw_control( sstr, x, y );
        }

        std::vector<unsigned> targets;
        target_lines( g, std::back_inserter( targets ) );
        std::vector<float> ys;
        std::transform( targets.begin(), targets.end(), std::back_inserter( ys ), settings.height - ( boost::lambda::_1 + 0.5 ) * settings.elem_height );
        ymin = std::min( ymin, *std::min_element( ys.begin(), ys.end() ) );
        ymax = std::max( ymax, *std::max_element( ys.begin(), ys.end() ) );
        settings.draw_targets( sstr, x, ys, g.type() );

        settings.draw_gate_line( os, x, ymin, ymax );

        x += settings.elem_width;
      }
      else
      {
        std::vector<float> yts;

        float y = settings.height - ( *g.begin_controls() + 0.5 ) * settings.elem_height;
        settings.draw_control( sstr, x, y );
        settings.draw_control( sstr, x + settings.elem_width, y );

        std::vector<unsigned> targets( g.begin_targets(), g.end_targets() );
        if ( boost::any_cast<peres_tag>( &g.type() )->swap_targets )
        {
          std::reverse( targets.begin(), targets.end() );
        }

        float yt1 = settings.height - ( targets.at( 0u ) + 0.5 ) * settings.elem_height;
        yts += yt1;
        settings.draw_control( sstr, x, yt1 );
        settings.draw_targets( sstr, x + settings.elem_width, yts, toffoli_tag() );

        float yt2 = settings.height - ( targets.at( 1u ) + 0.5 ) * settings.elem_height;
        yts.clear();
        yts += yt2;
        settings.draw_targets( sstr, x, yts, toffoli_tag() );

        settings.draw_gate_line( os, x, std::min( std::min( y, yt1 ), yt2 ), std::max( std::max( y, yt1 ), yt2 ) );
        settings.draw_gate_line( os, x + settings.elem_width, std::min( y, yt1 ), std::max( y, yt1 ) );

        settings.draw_peres_frame( os, x - settings.elem_width / 2, std::min( std::min( y, yt1 ), yt2 ) - settings.elem_height / 2,
                                   x + settings.elem_width + settings.elem_width / 2, std::max( std::max( y, yt1 ), yt2 ) + settings.elem_height / 2 );

        x += 2 * settings.elem_width;
      }

      os << sstr.str();
    }

    settings.draw_after( os );

    settings.draw_end( os );
  }

  void create_image( const std::string& filename, const circuit& circ, create_image_settings& settings )
  {
    std::filebuf fb;
    fb.open( filename.c_str(), std::ios::out );
    std::ostream os( &fb );
    create_image( os, circ, settings );
    fb.close();
  }
}
