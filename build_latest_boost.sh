#!/bin/bash

INSTALL_DIR=~/boost

wget http://downloads.sourceforge.net/project/boost/boost/1.60.0/boost_1_60_0.tar.bz2

tar --bzip2 -xf boost_1_60_0.tar.bz2
cd boost_1_60_0/
./bootstrap.sh --with-toolset=gcc --with-libraries=iostreams,filesystem,system --prefix=${INSTALL_DIR}
./b2 toolset=gcc-4.9 link=static threading=multi runtime-link=shared cxxflags=-fPIC
./b2 toolset=gcc-4.9 install --prefix=${INSTALL_DIR}


