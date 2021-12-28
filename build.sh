#!/bin/bash

set -e

mkdir -p cmake/build || echo "build folder exists, skipping"

./generate.sh

TARGET="all"
if [ ! -z "$1" ]; then
  TARGET=$1
fi

echo "building target: $TARGET"

cd cmake/build
cmake ../..
cmake --build . --target ${TARGET}

cd ../..

echo "BUILD - ALL GOOD"