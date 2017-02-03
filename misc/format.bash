#!/bin/bash

CURRDIR=`dirname $0`

clang-format-3.7 -i $CURRDIR/../src/* $CURRDIR/../src/*/* $CURRDIR/../test/*
