#!/usr/bin/python
import glob, re, os, sys

from PyQt4.QtGui import QApplication

from revkit import *
from revlib import *
from revlibui import *

if __name__ == "__main__":
    server = revlib_server()

    # import json
    # import urllib2

    # text = urllib2.urlopen( "http://www.revlib.org/doc/functions2.php" ).read()
    # functions = json.loads( text )
    # for f in functions:
    #     _id = server.get_function_id( f['abbreviation'] )
    #     if _id is not None:
    #         server.edit_function( _id, name = f['name'], abbreviation = f['abbreviation'], description = f['description'] )

    # TODO Inverse Peres
    # for real in glob.glob( "../../../../benchmarks/circuits/*.real" ):
    #     circ = circuit()
    #     s = read_realization( circ, real )
    #     if s == True:
    #         name = re.sub( '(-v.)?_\d+\.real', '', os.path.basename( real ) )
    #         nr = int( re.search( '_(\d+)\.real', real ).group( 1 ) )
    #         server.add_realization( name, circ, nr )
    #     else:
    #         print "Error in %s: %s" % ( real, s )

    a = QApplication([])

    w = FunctionsWidget()
    w.setItems( server.list_functions_by_categories() )
    w.show()

    sys.exit( a.exec_() )
