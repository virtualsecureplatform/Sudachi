#!/usr/bin/bash
echo "run client(input is randomly selected)"
./smallmul_client
echo "run Sudachi"
../../src/sudachi
echo "run verify"
./smallmul_verify