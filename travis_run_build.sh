mkdir build
cd build
cmake -DCMAKE_CXX_COMPILER=$COMPILER -DMOCKTURTLE_TEST=ON ..
make
