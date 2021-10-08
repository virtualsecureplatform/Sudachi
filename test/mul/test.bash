#!/usr/bin/bash
echo "run client(input is randomly selected)"
./mul_client
echo "run Sudachi"
../../src/sudachi
echo "run verify"
./mul_verify