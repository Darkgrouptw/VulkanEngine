rm -r build
mkdir build
cd build
cmake -S ../ -B ./
make clean
make -j8
make Shaders