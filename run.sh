#!/bin/bash

if [ -z "$1" ]; then
  echo "specify target";
  echo "- client";
  echo "- inner_server";
  echo "- outer_server";
  exit 1;
fi

export GRPC_DEFAULT_SSL_ROOTS_FILE_PATH=/usr/local/Cellar/grpc/1.41.0/share/grpc/roots.pem

./cmake/build/bin/$1
