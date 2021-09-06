#!/bin/bash

if [ "$#" -ne 1 ]; then
  echo "please specify either server or client"
  exit 1;
fi

if [ "$1" == "server" ]; then
  ./cmake/build/bin/example_server
elif [ "$1" == "client" ]; then
  ./cmake/build/bin/example_client
else
  echo "please specify either server or client"
  exit 1;
fi

