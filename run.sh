export LD_LIBRARY_PATH="$HOME"/.local/lib/:"$GCC_DIR"/lib64/:"$LD_LIBRARY_PATH"

cmake -S . -B ./build -DCMAKE_CXX_COMPILER="$GCC_DIR"/bin/g++ -DCMAKE_C_COMPILER="$GCC_DIR"/bin/gcc
make -C ./build -j8 --silent
./build/effective_modern_cpp
