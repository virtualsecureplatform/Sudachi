#!/usr/bin/bash
echo "run client(input is randomly selected)"
./Control_client
echo "run Sudachi"
../../src/sudachi
echo "run verify"
./Control_verify