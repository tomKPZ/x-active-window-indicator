#! /usr/bin/env bash

/usr/share/clang/run-clang-tidy.py . -fix -quiet

cppcheck src --inconclusive --enable=all --std=posix --max-configs=1 -Iinclude \
	 -I/usr/include --quiet 2>&1 | grep -v "(information)" 1>&2

clang-format -i src/*
clang-format -i include/*
