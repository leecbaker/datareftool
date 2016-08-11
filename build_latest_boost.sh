#!/bin/bash
set -e -u -x
INSTALL_DIR=~/boost

wget http://downloads.sourceforge.net/project/boost/boost/1.60.0/boost_1_60_0.tar.bz2

tar --bzip2 -xf boost_1_60_0.tar.bz2
cd boost_1_60_0/

if [ "$(uname)" == "Darwin" ]; then
BOOTSTRAP_FLAGS=""
B2_FLAGS=""
else
BOOTSTRAP_FLAGS="--with-toolset=gcc"
B2_FLAGS="toolset=gcc-4.9"
fi
./bootstrap.sh ${BOOTSTRAP_FLAGS} --with-libraries=iostreams,filesystem,system --prefix=${INSTALL_DIR}
./b2 ${B2_FLAGS} link=static threading=multi runtime-link=shared cxxflags=-fPIC
./b2 ${B2_FLAGS} install --prefix=${INSTALL_DIR}


