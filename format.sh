#!/usr/bin/env bash
set -euo pipefail

IGNORE_FILES=(
    "EgFs/test/EgFs_test/src/main.c"
    "EgGpusSdl/test/EgGpusSdl_test/src/main.c"
    "EgShapedraw/src/stb_truetype.h"
    "EgSpatialsSystems/test/EgSpatialsSystems_test/src/main.c"
    "EgSpirv/test/EgSpirv_test/src/main.c"
    "EgUi/test/EgUi_test/src/main.c"
    "cimgui/include/cimconfig.h"
    "cimgui/include/cimgui.h"
    "cimgui/src/imconfig.h"
    "cimgui/src/imgui.h"
    "cimgui/src/imgui_internal.h"
    "cimgui/src/imstb_rectpack.h"
    "cimgui/src/imstb_textedit.h"
    "cimgui/src/imstb_truetype.h"
    "egmath/test/egmath_test/src/main.c"
    "imgui/include/imconfig.h"
    "imgui/include/imgui.h"
    "imgui/src/imgui_internal.h"
    "imgui/src/imstb_rectpack.h"
    "imgui/src/imstb_textedit.h"
    "imgui/src/imstb_truetype.h"
)

IGNORE=( -not -name "bake_config.h" )
for f in "${IGNORE_FILES[@]}"; do
    IGNORE+=( -not -path "./$f" )
done

find . \( -name "*.c" -o -name "*.h" \) "${IGNORE[@]}" | xargs clang-format -i
