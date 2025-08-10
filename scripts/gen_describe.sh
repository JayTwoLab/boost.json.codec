#!/usr/bin/env bash
set -euo pipefail

DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$DIR/.." && pwd)"
SCRIPT="$ROOT/tools/gen_all_structs.py"

HEADER="${2:-$ROOT/model.hpp}"
OUT="${3:-$ROOT/describe_all.gen.hpp}"

# 1st arg or env var, else try common locations
LIB="${1:-${LIBCLANG_SO:-}}"
if [[ -z "${LIB}" ]]; then
  for c in \
    /usr/lib/llvm-*/lib/libclang.so \
    /usr/lib/libclang.so \
    /usr/local/opt/llvm/lib/libclang.dylib \
    /opt/homebrew/opt/llvm/lib/libclang.dylib ; do
    if [[ -f "$c" ]]; then LIB="$c"; break; fi
  done
fi

[[ -f "$SCRIPT" ]] || { echo "Missing $SCRIPT"; exit 1; }
[[ -n "${LIB}" ]]  || { echo "Usage: $(basename "$0") /path/to/libclang.so [header] [out]"; exit 1; }

PYTHONUTF8=1 python3 "$SCRIPT" --libclang "$LIB" --out "$OUT" --std c++20 -I "$ROOT" "$HEADER"
echo "Generated $OUT"
