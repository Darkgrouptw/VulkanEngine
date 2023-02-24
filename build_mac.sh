cd build
cmake -S ../ -B ./ -DCMAKE_C_COMPILER=/usr/bin/gcc -DCMAKE_CXX_COMPILER=/usr/bin/g++
make clean
make
make Shaders