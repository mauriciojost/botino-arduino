#!/bin/bash

echo "This script aims to help adding the includes in the setup of Intellij."

CURRDIR=$(readlink -e `dirname $0`)

cd $CURRDIR/..

echo "Cleaning environment..."
rm -fr .pioenvs
rm libraries.list
rm includes.list

echo "Listing libraries..."
platformio run --verbose > run.logs
cat run.logs | grep g++ | tr ' ' '\n' | grep '\-I' | sort | uniq | sed 's/-I//g' > libraries.list

while read -r line
do
  echo '<include-dir path="'$line'"/>' >> includes.list
done < libraries.list
rm libraries.list

echo "Done. Add the following includes configuration lines under tag \"component name=\"CppTools.Loader\"\", in .idea/misc.xml IDEA configuration file:"
 
cat includes.list

rm libraries.list
rm includes.list

