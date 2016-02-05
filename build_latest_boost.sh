#!/bin/bash

INSTALL_DIR=~/boost

wget http://downloads.sourceforge.net/project/boost/boost/1.60.0/boost_1_60_0.tar.bz2

tar --bzip2 -xf boost_1_60_0.tar.bz2
cd boost_1_60_0/
./bootstrap.sh --with-libraries=iostreams,filesystem,system --prefix=${INSTALL_DIR}
./b2 link=static threading=multi runtime-link=shared
./b2 install --prefix=${INSTALL_DIR}


