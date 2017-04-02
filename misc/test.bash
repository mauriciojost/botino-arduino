#!/bin/bash

set -x
set -e

CURRDIR=`readlink -e $(dirname $0)`

cd $CURRDIR/..

echo $PATH
which platformio
which pio 

platformio test -e param-stream-test
platformio run
