#!/bin/bash

set -e

mkdir -p cmake/build || echo "build folder exists, skipping"

./generate.sh

cd cmake/build
cmake ../..
make

cd ../..

echo "ALL GOOD"