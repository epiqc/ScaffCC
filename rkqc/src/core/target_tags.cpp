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

#include "target_tags.hpp"

namespace revkit
{

  bool same_type( const gate& g1, const gate& g2 )
  {
    return g1.type().type() == g2.type().type();
  }

  bool is_toffoli( const gate& g )
  {
    return is_type<toffoli_tag>( g.type() );
  }
  bool is_cnot( const gate& g )
  {
    return is_type<cnot_tag>( g.type() );
  }

  bool is_not( const gate& g )
  {
    return is_type<not_tag>( g.type() );
  }

  bool is_fredkin( const gate& g )
  {
    return is_type<fredkin_tag>( g.type() );
  }

  bool is_peres( const gate& g )
  {
    return is_type<peres_tag>( g.type() );
  }

    bool is_v( const gate& g )
  {
    return is_type<v_tag>( g.type() );
  }

  bool is_vplus( const gate& g )
  {
    return is_type<vplus_tag>( g.type() );
  }

  bool is_module( const gate& g )
  {
    return is_type<module_tag>( g.type() );
  }

}
