# RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
# Copyright (C) 2009-2011  The RevKit Developers <revkit@informatik.uni-bremen.de>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import signal

def circuit_add_runtime( circ, runtime ):
    try:
        r = circ.runtime
    except AttributeError:
        r = 0.0
    circ.runtime = r + runtime

def circuit_set_name( circ, filename ):
    circ.circuit_name = filename[filename.rfind('/') + 1:filename.rfind('.')]

class TimeoutException( Exception ):
    pass

class Timeout():
    pass

def timeout( timeout_time ):
    def timeout_function( f ):
        def f2( *args, **kwargs ):
            def timeout_handler( signum, frame ):
                raise TimeoutException()

            old_handler = signal.signal( signal.SIGALRM, timeout_handler )
            signal.alarm( timeout_time )

            try:
                retval = f( *args, **kwargs )
            except TimeoutException:
                return Timeout()
            finally:
                signal.signal( signal.SIGALRM, old_handler )
            signal.alarm( 0 )
            return retval
        return f2
    return timeout_function
