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

#include "pattern.hpp"

#include <boost/assign/std/vector.hpp>

using namespace boost::assign;

namespace revkit
{

  class pattern::priv
  {
  public:
    priv() {}

    initializer_map initializers;
    std::vector<std::string> inputs;
    std::vector<std::vector<unsigned> > patterns;
  };

  pattern::pattern()
    : d( new priv() )
  {
  }

  pattern::~pattern()
  {
    delete d;
  }

  void pattern::add_initializer( const std::string& name, unsigned value )
  {
    d->initializers[name] = value;
  }

  void pattern::add_input( const std::string& name )
  {
    d->inputs += name;
  }

  void pattern::add_pattern( const std::vector<unsigned>& pattern )
  {
    d->patterns += pattern;
  }

  const pattern::initializer_map& pattern::initializers() const
  {
    return d->initializers;
  }

  const std::vector<std::string>& pattern::inputs() const
  {
    return d->inputs;
  }

  const pattern::pattern_vec& pattern::patterns() const
  {
    return d->patterns;
  }

}
