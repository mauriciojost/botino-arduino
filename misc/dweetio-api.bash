#!/bin/bash

set -e
set -x 

export PRETTY='python -m json.tool'

curl -s -X POST 'http://dweet.io/dweet/for/dev0-target?led1.on=0' | $PRETTY

curl -s -X GET 'http://dweet.io/get/latest/dweet/for/dev0' | $PRETTY
