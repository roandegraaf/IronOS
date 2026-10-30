#!/bin/bash
# Render the TS101 128x32 screens to PNGs on the host, without hardware.
#
# Usage: scripts/preview_ts101.sh [--no-gen] [name-filter]
#
# Writes build/preview/<screen>--<fixture>--<rh|lh>.png (512x128, 4x, white on black).
# Fixtures live in tools/preview/fixtures.h. The EN translation and settings tables are
# generated in the Docker builder; --no-gen reuses the existing source/Core/Gen files.
# name-filter only renders files whose name contains it (e.g. "soldering_simplified").

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SRC="$REPO_ROOT/source"
TOOL="$REPO_ROOT/tools/preview"
OUT="$REPO_ROOT/build/preview"
OBJ="$OUT/obj"
MODEL="TS101"
LANG_CODE="EN"
GEN_TRANSLATION="Core/Gen/Translation.$LANG_CODE.cpp"
GEN_SETTINGS="Core/Gen/settings_gen.$MODEL.cpp"
RUN_GEN=1
FILTER=""

die() {
    echo "error: $*" >&2
    exit 1
}

usage() {
    sed -n '2,9s/^# \{0,1\}//p' "${BASH_SOURCE[0]}"
}

for arg in "$@"; do
    case "$arg" in
    --no-gen) RUN_GEN=0 ;;
    -h | --help)
        usage
        exit 0
        ;;
    -*)
        usage >&2
        die "unknown argument: $arg"
        ;;
    *) FILTER="$arg" ;;
    esac
done

generate_sources() {
    command -v docker > /dev/null || die "docker not found; install Docker Desktop or pass --no-gen"
    docker info > /dev/null 2>&1 || die "Docker is not running; start Docker Desktop or pass --no-gen"
    # macros.txt only depends on the Makefile, so it goes stale when configuration.h changes.
    if [ -f "$SRC/Core/Gen/macros.txt" ] && [ "$SRC/Core/BSP/Miniware/configuration.h" -nt "$SRC/Core/Gen/macros.txt" ]; then
        rm -f "$SRC/Core/Gen/macros.txt"
    fi
    echo "==> Generating $GEN_TRANSLATION and $GEN_SETTINGS in Docker"
    # The builder image fails to start with macOS's default staff GID (20).
    (cd "$REPO_ROOT" && HOST_UID="$(id -u)" HOST_GID=1000 docker compose -f Env.yml run --rm builder \
        bash -c "cd source && make model=$MODEL $GEN_TRANSLATION $GEN_SETTINGS" > /dev/null) ||
        die "generating sources failed"
}

if [ "$RUN_GEN" = 1 ]; then
    generate_sources
fi
[ -f "$SRC/$GEN_TRANSLATION" ] && [ -f "$SRC/$GEN_SETTINGS" ] || die "generated sources missing; run without --no-gen"

SOURCES=(
    "$TOOL/main.cpp"
    "$TOOL/render.cpp"
    "$TOOL/stubs/preview_stubs.cpp"
    "$SRC/Core/Drivers/OLED.cpp"
    "$SRC/Core/Src/Settings.cpp"
    "$SRC/Core/Src/settingsGUI.cpp"
    "$SRC/Core/Src/ScrollMessage.cpp"
    "$SRC/Core/Src/Translation.cpp"
    "$SRC/Core/LangSupport/lang_single.cpp"
    "$SRC/Core/Threads/UI/logic/SettingsMenu.cpp"
    "$SRC/Core/Threads/UI/logic/utils/SolderingCommon.cpp"
    "$SRC/Core/Threads/UI/logic/utils/getSleepTimeout.cpp"
    "$SRC/Core/Threads/UI/logic/utils/min.cpp"
    "$SRC/$GEN_TRANSLATION"
    "$SRC/$GEN_SETTINGS"
)
for f in "$SRC"/Core/Threads/UI/drawing/mono_128x32/*.cpp; do
    # The debug menu reads FreeRTOS task internals the stubs don't model.
    [ "$(basename "$f")" = "draw_debug_menu.cpp" ] || SOURCES+=("$f")
done

INCLUDES=(-I"$TOOL" -I"$TOOL/stubs" -I"$TOOL/stubs/include" -I"$SRC/Core/Inc" -I"$SRC/Core/BSP" -I"$SRC/Core/BSP/Miniware")
while IFS= read -r dir; do
    INCLUDES+=(-I"$dir")
done < <(find "$SRC/Core/Threads" -type d)
INCLUDES+=(-I"$SRC/Core/Drivers" -I"$SRC/Core/Drivers/usb-pd/include")

# Settings.cpp otherwise memcmp()s the settings against their STM32 flash address.
CXXFLAGS=(-std=gnu++17 -O1 -fshort-wchar "-DMODEL_$MODEL" -DCANT_DIRECT_READ_SETTINGS -w)

echo "==> Compiling the drawing code with host clang"
rm -rf "$OBJ"
mkdir -p "$OBJ"
pids=()
objects=()
for src in "${SOURCES[@]}"; do
    obj="$OBJ/$(basename "${src%.cpp}").o"
    objects+=("$obj")
    clang++ "${CXXFLAGS[@]}" "${INCLUDES[@]}" -c "$src" -o "$obj" &
    pids+=($!)
done
for pid in "${pids[@]}"; do
    wait "$pid" || die "compile failed"
done
clang++ "${objects[@]}" -lz -o "$OUT/preview" || die "link failed"

echo "==> Rendering"
find "$OUT" -maxdepth 1 -name '*.png' ${FILTER:+-name "*$FILTER*"} -delete
"$OUT/preview" "$OUT" ${FILTER:+"$FILTER"}
