#!/usr/bin/env bash
set -euo pipefail

readonly CIMGUI_TAG="1.92.9bdock"
readonly IMGUI_TAG="v1.92.9b-docking"
readonly CIMGUI_REPOSITORY="https://github.com/cimgui/cimgui.git"
readonly IMGUI_REPOSITORY="https://github.com/ocornut/imgui.git"

script_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
workspace_dir=$(cd -- "$script_dir/../../.." && pwd)
temporary_dir=$(mktemp -d)
stage_dir=$(mktemp -d "$script_dir/.upstream.XXXXXX")

cleanup() {
    rm -rf "$temporary_dir" "$stage_dir"
}
trap cleanup EXIT

git clone --quiet --depth 1 --branch "$CIMGUI_TAG" "$CIMGUI_REPOSITORY" "$temporary_dir/cimgui"
git clone --quiet --depth 1 --branch "$IMGUI_TAG" "$IMGUI_REPOSITORY" "$temporary_dir/imgui"

cimgui_version=$(sed -n 's|.*imgui.h file version "\([^"]*\)".*|\1|p' "$temporary_dir/cimgui/cimgui.cpp")
imgui_version=$(sed -n 's|#define IMGUI_VERSION       "\([^"]*\)"|\1|p' "$temporary_dir/imgui/imgui.h")

if [[ -z "$cimgui_version" || "$cimgui_version" != "$imgui_version" ]]; then
    printf 'Version mismatch: cimgui=%s imgui=%s\n' "$cimgui_version" "$imgui_version" >&2
    exit 1
fi

mkdir -p "$stage_dir/include" "$stage_dir/src"
cp "$temporary_dir/cimgui/cimgui.h" "$temporary_dir/cimgui/cimconfig.h" "$stage_dir/include/"
cp "$temporary_dir/cimgui/cimgui.cpp" "$stage_dir/src/"
cp "$temporary_dir/imgui"/{imconfig.h,imgui.h,imgui_internal.h,imstb_rectpack.h,imstb_textedit.h,imstb_truetype.h,imgui.cpp,imgui_demo.cpp,imgui_draw.cpp,imgui_tables.cpp,imgui_widgets.cpp} "$stage_dir/src/"

# Upstream places ImGui under imgui/, but Bake compiles the direct files in src/.
# Keep cimgui.cpp aligned with the flattened Bake source layout.
sed -i \
    -e 's|#include "\./imgui/imgui.h"|#include "imgui.h"|' \
    -e 's|#include "\./imgui/imgui_internal.h"|#include "imgui_internal.h"|' \
    "$stage_dir/src/cimgui.cpp"

rm -rf "$script_dir/src"
mv "$stage_dir/src" "$script_dir/src"
cp "$stage_dir/include/cimgui.h" "$stage_dir/include/cimconfig.h" "$script_dir/include/"

cd "$workspace_dir"
bake rebuild vendor/eg/cimgui
