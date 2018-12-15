ln -s /usr/bin/gcc-7 /usr/local/bin/gcc
ln -s /usr/bin/g++-7 /usr/local/bin/g++
export CC=/usr/bin/gcc-7
export CXX=/usr/bin/g++-7
gcc -v && g++ -v && cmake --version
mkdir build
cd build
cmake -DCMAKE_CXX_FLAGS=-fPIC -DCMAKE_C_FLAGS=-fPIC ..
make
