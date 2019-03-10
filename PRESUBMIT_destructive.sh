#! /usr/bin/env bash

cmake . -DCMAKE_EXPORT_COMPILE_COMMANDS=On

iwyu-tool -p . -j8 -- --mapping_file=iwyu.imp

/usr/share/clang/run-clang-tidy.py . -fix -quiet

cppcheck src --inconclusive --enable=all --std=posix --max-configs=1 -Iinclude \
	 -I/usr/include --quiet 2>&1 | grep -v "(information)" 1>&2

clang-format -i src/* include/*

cpplint --filter='-build/include,-build/header_guard,
	-whitespace/parens,-readability/check' \
	--quiet include/* src/*
