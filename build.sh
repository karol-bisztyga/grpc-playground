#!/bin/bash

set -e

mkdir cmake/build || echo "build folder exists, skipping"
cd cmake/build
cmake ../..
make

cd ../..

echo "ALL GOOD"