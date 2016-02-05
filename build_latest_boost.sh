#!/bin/bash

wget http://downloads.sourceforge.net/project/boost/boost/1.60.0/boost_1_60_0.tar.bz2

tar --bzip2 -xf boost_1_60_0.tar.bz2
cd boost_1_60_0/
./bootstrap.sh --with-libraries=iostreams,filesystem,system --prefix=~/boost
./b2 link=static threading=multi runtime-link=shared
./b2 install --prefix=~/boost


