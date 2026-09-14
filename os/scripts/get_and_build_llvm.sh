#!/bin/sh
git clone --depth 1 https://github.com/swiftlang/llvm-project.git
cd llvm-project

cmake -G Ninja -S llvm -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DLLVM_ENABLE_ASSERTIONS=OFF \
  -DLLVM_ENABLE_PROJECTS="clang" \
  -DLLVM_TARGETS_TO_BUILD="X86" \
  -DLLVM_INCLUDE_TESTS=OFF \
  -DLLVM_INCLUDE_EXAMPLES=OFF \
  -DLLVM_INCLUDE_BENCHMARKS=OFF \
  -DLLVM_INCLUDE_DOCS=OFF \
  -DCLANG_INCLUDE_TESTS=OFF \
  -DCLANG_INCLUDE_DOCS=OFF \
  -DLLVM_PARALLEL_LINK_JOBS=2 \
  -DCMAKE_INSTALL_PREFIX="$HOME/toolchain/x86_64-bounds-safety"

ninja -C build clang -j"$(nproc)"
ninja -C build install-clang install-clang-resource-headers
