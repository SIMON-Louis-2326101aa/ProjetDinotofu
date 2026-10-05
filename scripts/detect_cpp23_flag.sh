#!/usr/bin/env bash
set -euo pipefail
CXX_BIN="${1:-${CXX:-g++}}"
probe() {
    printf 'int main(){return 0;}\n' | "$CXX_BIN" -x c++ "$1" -fsyntax-only - >/dev/null 2>&1
}
if probe -std=c++23; then
    printf '%s\n' '-std=c++23'
elif probe -std=c++2b; then
    # Older GCC/MinGW releases expose the C++23 draft under the historical c++2b name.
    printf '%s\n' '-std=c++2b'
else
    echo "Le compilateur '$CXX_BIN' ne prend pas en charge C++23 (-std=c++23 / -std=c++2b)." >&2
    exit 1
fi
