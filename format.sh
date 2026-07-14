#!/usr/bin/env bash
set -euo pipefail

find . \( -name "*.c" -o -name "*.h" \) -not -name "bake_config.h" | xargs clang-format -i
