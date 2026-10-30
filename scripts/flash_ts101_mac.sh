#!/bin/bash
# Build the TS101 EN firmware in Docker and flash it through the Miniware DFU disk on macOS.
#
# Usage: scripts/flash_ts101_mac.sh [--skip-build]
#
# Environment overrides:
#   DFU_VOLUME          mount point of the DFU disk       (default /Volumes/TS101_DFU)
#   DFU_WAIT_SECONDS    how long to wait for the DFU disk  (default 120)
#   FLASH_WAIT_SECONDS  how long to wait for .RDY/.ERR     (default 60)

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MODEL="TS101"
LANG_CODE="EN"
HEX_NAME="${MODEL}_${LANG_CODE}"
HEX_FILE="$REPO_ROOT/source/Hexfile/$HEX_NAME.hex"
DFU_VOLUME="${DFU_VOLUME:-/Volumes/TS101_DFU}"
DFU_WAIT_SECONDS="${DFU_WAIT_SECONDS:-120}"
FLASH_WAIT_SECONDS="${FLASH_WAIT_SECONDS:-60}"
SKIP_BUILD=0

die() {
    echo "error: $*" >&2
    exit 1
}

usage() {
    sed -n '2,9s/^# \{0,1\}//p' "${BASH_SOURCE[0]}"
}

for arg in "$@"; do
    case "$arg" in
    --skip-build) SKIP_BUILD=1 ;;
    -h | --help)
        usage
        exit 0
        ;;
    *)
        usage >&2
        die "unknown argument: $arg"
        ;;
    esac
done

[ "$(uname -s)" = "Darwin" ] || die "this script is for macOS; use scripts/flash_ts10X_linux.sh on Linux"

build_firmware() {
    command -v docker > /dev/null || die "docker not found; install Docker Desktop"
    docker info > /dev/null 2>&1 || die "Docker is not running; start Docker Desktop and retry"
    echo "==> Building $HEX_NAME.hex in Docker"
    # The builder image fails to start with macOS's default staff GID (20).
    (cd "$REPO_ROOT" && HOST_UID="$(id -u)" HOST_GID=1000 docker compose -f Env.yml run --rm builder \
        bash -c "cd source && make -j8 model=$MODEL firmware-$LANG_CODE") ||
        die "firmware build failed"
}

validate_hex() {
    [ -f "$HEX_FILE" ] || die "$HEX_FILE not found; run without --skip-build"
    if [ "$(head -c1 "$HEX_FILE")" != ":" ] || [ "$(tail -n1 "$HEX_FILE" | head -c1)" != ":" ]; then
        die "$HEX_FILE does not look like an Intel HEX file"
    fi
}

wait_for_volume() {
    local waited=0
    if [ ! -d "$DFU_VOLUME" ]; then
        cat << EOF
==> Waiting up to ${DFU_WAIT_SECONDS}s for $DFU_VOLUME
    Unplug the iron, hold the front button (closest to the tip),
    and plug it into this Mac with a USB data cable.
EOF
    fi
    while [ ! -d "$DFU_VOLUME" ]; do
        if [ "$waited" -ge "$DFU_WAIT_SECONDS" ]; then
            die "DFU volume $DFU_VOLUME did not appear within ${DFU_WAIT_SECONDS}s.
       Check the cable carries data, hold the front button while plugging in,
       or set DFU_VOLUME if the disk mounts under another name."
        fi
        sleep 1
        waited=$((waited + 1))
    done
    echo "==> Found $DFU_VOLUME"
}

file_on_volume() {
    find "$DFU_VOLUME" -maxdepth 1 -type f -iname "$1" 2> /dev/null | head -n1
}

find_result() {
    local f
    f="$(file_on_volume "$HEX_NAME.rdy")"
    [ -n "$f" ] || f="$(file_on_volume "$HEX_NAME.err")"
    [ -n "$f" ] && echo "$f"
}

hex_still_on_volume() {
    [ -n "$(file_on_volume "$HEX_NAME.hex")" ]
}

flash() {
    echo "==> Copying $HEX_NAME.hex to $DFU_VOLUME"
    cp -X "$HEX_FILE" "$DFU_VOLUME/$HEX_NAME.hex" || die "copy to $DFU_VOLUME failed"
    sync

    echo "==> Waiting up to ${FLASH_WAIT_SECONDS}s for the bootloader to rename the file"
    local waited=0 result=""
    while [ "$waited" -lt "$FLASH_WAIT_SECONDS" ]; do
        if [ -d "$DFU_VOLUME" ] && ! hex_still_on_volume && result="$(find_result)"; then
            break
        fi
        result=""
        sleep 1
        waited=$((waited + 1))
    done

    local status=0
    case "$result" in
    *.[Rr][Dd][Yy])
        echo "==> RDY: flash succeeded ($(basename "$result"))"
        ;;
    *.[Ee][Rr][Rr])
        echo "==> ERR: the bootloader rejected the firmware ($(basename "$result")); re-enter DFU mode and retry" >&2
        status=1
        ;;
    *)
        echo "==> No .RDY/.ERR after ${FLASH_WAIT_SECONDS}s; volume contents:" >&2
        ls -la "$DFU_VOLUME" >&2 2> /dev/null || echo "    ($DFU_VOLUME is not mounted)" >&2
        status=1
        ;;
    esac

    if [ -d "$DFU_VOLUME" ]; then
        echo "==> Ejecting $DFU_VOLUME"
        diskutil eject "$DFU_VOLUME" || echo "warning: eject failed; eject $DFU_VOLUME manually" >&2
    fi

    if [ "$status" -eq 0 ]; then
        echo "==> Done. Unplug the iron and power it up."
    fi
    return "$status"
}

if [ "$SKIP_BUILD" -eq 0 ]; then
    build_firmware
fi
validate_hex
wait_for_volume
flash
