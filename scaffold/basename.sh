#!/bin/bash

# Yes, a basename util does exist, but it is the same between Ubuntu and Debian (go figure)
# No, existance is not checked; that is handled by the programs that use this as an input
var=${1##*/}
var=${var%%.scaffold}
echo ${var%%.scaf}
