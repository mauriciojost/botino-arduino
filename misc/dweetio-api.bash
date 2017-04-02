#!/bin/bash

set -e
set -x 

curl -H 'Content-Type:application/json' -X POST https://dweet.io:443/dweet/for/dev0 -D '{"hola":"campeon2"}'
