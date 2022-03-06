#!/bin/bash -x

set -euxo pipefail

apt update -y
apt install -y --no-install-recommends  \
  llvm-12 \
  python3-pip \
  clang-12 \
  cmake \
  make \
  libc++-12-dev \
  libc++abi1-12 \
  libc++abi-12-dev \
  libunwind-12-dev \
  ninja-build \
  lld-12

pip install conan --upgrade

export CC=/usr/bin/clang-12
export CXX=/usr/bin/clang++-12

mkdir build
cd build
conan install .. --build=missing --profile ../conanprofile.txt
cmake --verbose -GNinja -DCMAKE_C_COMPILER=clang-12 -DCMAKE_CXX_COMPILER=clang++-12 -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="$(llvm-config-12 "--prefix")" ..

ninja inverter-cli
ninja install
