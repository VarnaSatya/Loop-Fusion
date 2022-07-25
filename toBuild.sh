cmake -G "Ninja" -DCMAKE_INSTALL_PREFIX=../install -DCMAKE_BUILD_TYPE="Debug" -DLLVM_TARGETS_TO_BUILD="host" -DBUILD_SHARED_LIBS=1 -DLLVM_ENABLE_PROJECTS="clang" -DCMAKE_CXX_FLAGS=" -ggdb3 -gdwarf-4 "  ../llvm-project/llvm 

ninja install

cmake -G "Ninja" -DCMAKE_INSTALL_PREFIX=../install -DCMAKE_BUILD_TYPE="Debug" -DLLVM_TARGETS_TO_BUILD="host" -DBUILD_SHARED_LIBS=1 -DLLVM_ENABLE_PROJECTS="clang" -DCMAKE_CXX_FLAGS=" -ggdb3 -gdwarf-4 "  ../llvm-project/llvm 

ninja install
