 #!/bin/bash
 sudo add-apt-repository -y ppa:boost-latest/ppa
 sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
 sudo add-apt-repository -y ppa:h-rayflood/llvm-upper
 sudo apt-get update
 sudo apt-get install
 sudo apt-get install cmake
 if [ "$CXX" = "g++" ]; then sudo apt-get install gcc-4.9 g++-4.9; fi
 if [ "$CXX" = "clang++" ]; then sudo apt-get install clang-3.6; fi
 sudo apt-get install libboost1.54-dev libboost-filesystem1.54-dev libboost-iostreams1.54-dev libboost-system1.54-dev
 sudo apt-get install libx11-dev # libboost-all-dev
 if [ "$CC" = "gcc" ]; then CC=gcc-4.9; fi
 if [ "$CC" = "clang" ]; then CC=clang-3.6; fi
 if [ "$CXX" = "g++" ]; then CXX=g++-4.9; fi
 if [ "$CXX" = "clang++" ]; then CXX=clang++-3.6; fi
 cmake --version
 $CC --version
 $CXX --version