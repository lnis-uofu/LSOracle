mkdir build
cd build
cmake -DCMAKE CXX FLAGS=-fPIC -DCMAKE C FLAGS=-fPIC ..
make
