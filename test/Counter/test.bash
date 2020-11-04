#!/usr/bin/bash
echo "run client(input is randomly selected)"
./Counter_client
echo "run Sudachi"
../../src/sudachi
echo "run verify"
./Counter_verify