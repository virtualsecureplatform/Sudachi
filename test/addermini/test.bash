#!/usr/bin/bash
echo "run client(input is randomly selected)"
./addermini_client
echo "run Sudachi"
../../src/sudachi
echo "run verify"
./addermini_verify