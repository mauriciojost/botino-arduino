#!/bin/bash

set -e
set -x

function run_test() {

  export main_src="$1"

  export flags="$2"

  export headers=$(find * -name "*.h" | xargs -I% dirname % | uniq | grep -v test | grep -v unity | xargs -I% echo "-I %")
  export src_c=$(find * -name "*.c" | grep -v test | grep -v unity | xargs -I% echo " %")
  export src_cpp=$(find * -name "*.cpp" | grep -v test | grep -v unity | xargs -I% echo " %")

  export headers_unity="-I libs/unity/src "
  export src_unity="libs/unity/src/*.c"

  g++ -o simulator.bin $flags $src_c $src_cpp $src_unity $main_src $headers $headers_unity

  ./simulator.bin

  rm ./simulator.bin

}

general_flags="-D UNIT_TEST -D SUBCYCLES_2 -D LOG_LEVEL=0 -D BINARY_LEVEL"

for f in $(find test -name *.cpp)
do
  echo ""
  echo ""
  echo "### Running $f"
  echo ""
  echo ""
  run_test "$f" "$general_flags"
done
