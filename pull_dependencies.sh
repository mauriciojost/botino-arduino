#!/bin/bash

# This script will pull the dependency libraries from GitHub
# using dependencies settings with platformio syntax: 
# - esp8266/Arduino#master
# - mauriciojost/log4ino-arduino#1.0.0
# - git@bitbucket.org:mauriciojost/main4ino-arduino.git#master
# In an attempt to stop using platformio (community not big enough yet).

ROOT_DIR=`dirname $(readlink -e $0)`
CONFIG_FILE=$1
LIBS_DIR=$ROOT_DIR/lib_deps_external
SRC_DIR=$ROOT_DIR/src

function pull_dependency() {
  local d="$1"
  if [[ "$d" =~ ^(.*)@(.*git)#(.*)$ ]]; 
  then 
    local giturl=${BASH_REMATCH[1]}@${BASH_REMATCH[2]}
    local gitbranch=${BASH_REMATCH[3]}
  elif [[ "$d" =~ ^(.*)/(.*)#(.*)$ ]]; 
  then 
    local ghuser=${BASH_REMATCH[1]}
    local ghproject=${BASH_REMATCH[2]}
    local giturl="https://github.com/$ghuser/$ghproject.git"
    local gitbranch=${BASH_REMATCH[3]}
  else 
    echo "Unkown syntax, discarding: $d"
  fi
  echo "Pulling: $giturl / $gitbranch"
  cd $LIBS_DIR
  git clone -b "$gitbranch" "$giturl"
}

function link_dependency() {
  local dep_dir="$1"
  local src_dir="$2"
  echo "Linking $dep_dir/src/* to $src_dir..."
  ln -sf $dep_dir/src/* $src_dir/
}


source $CONFIG_FILE

rm -fr "$LIBS_DIR"
mkdir -p "$LIBS_DIR"
for dep in $LIB_DEPS_EXTERNAL
do
  pull_dependency "$dep"
done
for dep in `find $LIBS_DIR -maxdepth 1 -mindepth 1 -type d`
do
  link_dependency "$dep" "$SRC_DIR"
done
