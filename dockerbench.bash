#!/bin/bash

sudo docker pull ghcr.io/virtualsecureplatform/yosysbt:latest
sudo docker run -v $(pwd):/Sudachi ghcr.io/virtualsecureplatform/yosysbt:latest bash -c "cd /Sudachi && bash bench.bash"