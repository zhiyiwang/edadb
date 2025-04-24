#!/bin/bash
rm -f *.db 
make -j 2>&1 | tee make_output.txt 

# make -j > make_output.txt 2>&1
