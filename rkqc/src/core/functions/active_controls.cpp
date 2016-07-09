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

#include "active_controls.hpp"

#include <boost/bind.hpp>

namespace revkit
{
  class active_controls::priv
  {
  public:
    priv() {}
    
    gate::line_container _active_controls;
  };

  active_controls::active_controls()
    : d( new priv() )
  {
  }
  
  active_controls::~active_controls()
  {
    //delete d;
  }
  
  void active_controls::add( gate::line control )
  {
    d->_active_controls.insert( control );
  }

  void active_controls::remove( gate::line control )
  {
    d->_active_controls.erase( control );
  }

  const gate::line_container& active_controls::controls() const
  {
    return d->_active_controls;
  }

  void active_controls::operator()( gate& g ) const
  {
    std::for_each( d->_active_controls.begin(), d->_active_controls.end(), boost::bind( &gate::add_control, &g, _1 ) );
  }

}

