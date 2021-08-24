#!/bin/bash

set -e

rm bin/* || echo "no binaries, skipping"
cmake . && make -j
