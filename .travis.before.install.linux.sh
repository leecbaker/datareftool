 #!/bin/bash
 sudo add-apt-repository -y ppa:boost-latest/ppa
 sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
 sudo add-apt-repository -y ppa:h-rayflood/llvm-upper
 sudo apt-get update
 sudo apt-get install
 sudo apt-get install cmake
 if [ "$CXX" = "g++" ]; then sudo apt-get install gcc-4.9 g++-4.9; fi
 if [ "$CXX" = "clang++" ]; then sudo apt-get install clang-3.6; fi
 sudo update-alternatives --set gcc `which gcc-4.9`
 sudo update-alternatives --set g++ `which g++-4.9`
 sudo update-alternatives --set clang `which clang-3.6`
 sudo update-alternatives --set clang++ `which clang++-3.6`
 sudo apt-get install libboost1.54-dev libboost-filesystem1.54-dev libboost-iostreams1.54-dev libboost-system1.54-dev
 sudo apt-get install libx11-dev # libboost-all-dev
 cmake --version
 $CC --version
 $CXX --version