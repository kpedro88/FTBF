#!/bin/bash

FILE=`echo $1`

root -b -l -q 'makeTrees.C+("'${FILE}'")'
