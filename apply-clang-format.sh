#!/bin/bash

find . -name "*.h" -o -name "*.cpp" -o -name "*.hpp" -o -name "*.c" | xargs clang-format -i

