#!/bin/bash
PROJ_DIR=$(pwd)
DATE=$(date +'%Y%m%d%H%M%S')
mkdir -p log/$DATE
cmake . -G Ninja -B /tmp/build -DENABLE_TEST=ON  -DUSE_AVX512=ON -DUSE_CONCRETE=ON
cd /tmp/build
# run twice to make sure to avoid the sbt conflict
ninja
ninja 
cd test/iscas85
for circuit in c17 c432 c499 c880 c1355 c1908 c2670 c3540 c5315 c6288 c7552; do
    cd $circuit
    bash test.bash | tee ${PROJ_DIR}/log/${DATE}/$circuit.log
    cd ..
    rm -r $circuit
done
