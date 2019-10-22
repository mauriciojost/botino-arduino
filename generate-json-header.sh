#!/bin/bash

echo "#define VERSION_DESCRIPTION_JSON \"" > src/Description.json.h

cat src/Description.json | jq -c . | sed 's/\\/\\\\/g' | sed 's/"/\\"/g' >> src/Description.json.h

echo "\"" >> src/Description.json.h

cat src/Description.json.h | tr --delete '\n'  > src/Description.json.2.h 
