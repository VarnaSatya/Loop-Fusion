clang -S -emit-llvm test.c -Xclang -disable-O0-optnone

opt -mem2reg -loop-simplify -instcombine -instnamer -indvars test.ll -S -o out.ll

opt -load ${LLVM_HOME}/build/lib/LLVMHello.so -hello2 out.ll >/dev/null --enable-new-pm=0 
