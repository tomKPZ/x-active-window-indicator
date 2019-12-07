#! /usr/bin/env bash

cmake . \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=On \
      -DCMAKE_C_COMPILER=clang \
      -DCMAKE_CXX_COMPILER=clang++

iwyu-tool -p . -j8 -- -Xiwyu --mapping_file=iwyu.imp

/usr/share/clang/run-clang-tidy.py . -fix -quiet

cppcheck src --inconclusive --enable=all --force --suppress=unusedFunction \
  --suppress=missingIncludeSystem >/dev/null

clang-format -i src/*
cmake-format -i CMakeLists.txt

cpplint --filter='-build/include,-build/header_guard,
	-whitespace/parens,-readability/check' \
	--quiet src/*
