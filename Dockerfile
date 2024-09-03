# syntax = docker/dockerfile:1.3-labs
FROM ubuntu:24.04

LABEL maintainer="nindanaoto(Kotaro MATSUOKA) <matsuoka.kotaro@gmail.com>"

# install build dependencies
RUN apt-get update && apt-get upgrade -y
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential g++ libomp-dev cmake git libgoogle-perftools-dev verilator ninja-build

# yosys
RUN DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential clang bison flex libreadline-dev gawk tcl-dev libffi-dev git graphviz xdot pkg-config python3 libboost-system-dev libboost-python-dev libboost-filesystem-dev zlib1g-dev
RUN git clone --recursive https://github.com/YosysHQ/yosys.git && cd yosys && make -j && make install && cd .. && rm -rf yosys

# sbt
RUN <<EOF
DEBIAN_FRONTEND=noninteractive apt-get install default-jre apt-transport-https curl gnupg -yqq
echo "deb https://repo.scala-sbt.org/scalasbt/debian all main" |  tee /etc/apt/sources.list.d/sbt.list
echo "deb https://repo.scala-sbt.org/scalasbt/debian /" |  tee /etc/apt/sources.list.d/sbt_old.list
curl -sL "https://keyserver.ubuntu.com/pks/lookup?op=get&search=0x2EE0EA64E40A89B84B2DF73499E82A75642AC823" | gpg --no-default-keyring --keyring gnupg-ring:/etc/apt/trusted.gpg.d/scalasbt-release.gpg --import
chmod 644 /etc/apt/trusted.gpg.d/scalasbt-release.gpg
apt-get update
apt-get install sbt
EOF