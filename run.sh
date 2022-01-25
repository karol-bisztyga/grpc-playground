#!/bin/bash

export GRPC_DEFAULT_SSL_ROOTS_FILE_PATH=/usr/local/Cellar/grpc/1.41.0/share/grpc/roots.pem

./cmake/build/bin/client $1

