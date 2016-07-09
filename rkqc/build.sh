#!/bin/bash

cd scripts/
echo "Building Dependencies..."
python ../make.py bootstrap > messages.log
echo "Building RKQC"
python ../make.py build > messages.log
grep -i "Error" messages.log > errors.log
echo "Done."

