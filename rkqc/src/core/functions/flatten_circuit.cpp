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

#include "flatten_circuit.hpp"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include "../circuit.hpp"
#include "../target_tags.hpp"

#include "copy_metadata.hpp"

#define foreach BOOST_FOREACH

namespace revkit
{

  void flatten_circuit( const circuit& base, circuit& circ, bool keep_meta_data )
  {
    copy_metadata_settings settings;
    settings.copy_modules = keep_meta_data;
    settings.copy_inputbuses = keep_meta_data;
    settings.copy_outputbuses = keep_meta_data;
    settings.copy_statesignals = keep_meta_data;
    copy_metadata( base, circ, settings );

    foreach ( const gate& g, base )
    {
      if ( is_module( g ) )
      {
        circuit flattened;
        const module_tag& tag = boost::any_cast<module_tag>( g.type() );
        flatten_circuit( *tag.reference.get(), flattened );

        std::vector<unsigned> target_lines( g.begin_targets(), g.end_targets() );

        foreach ( const gate& fg, flattened )
        {
          gate& new_gate = circ.append_gate();

          std::for_each( g.begin_controls(), g.end_controls(), boost::bind( &gate::add_control, &new_gate, _1 ) );

          for ( gate::const_iterator it = fg.begin_controls(); it != fg.end_controls(); ++it )
          {
            new_gate.add_control( target_lines.at( tag.target_sort_order.at( *it ) ) );
          }

          for ( gate::const_iterator it = fg.begin_targets(); it != fg.end_targets(); ++it )
          {
            new_gate.add_target( target_lines.at( tag.target_sort_order.at( *it ) ) );
          }

          new_gate.set_type( fg.type() );
        }
      }
      else
      {
        circ.append_gate() = g;
      }
    }
  }

}


