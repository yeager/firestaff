#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -gt 1 ]; then
    printf 'usage: %s [MEDNAFEN_1.32.1_SOURCE]\n' "$0" >&2
    exit 2
fi

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
source_root=${1:-/tmp/mednafen-src}
sdl2_prefix=${FIRESTAFF_MEDNAFEN_SDL2_PREFIX:-}
# An explicit build root keeps parallel local investigations from reusing an
# instrumented binary produced from a different patch revision.
build_root=${FIRESTAFF_MEDNAFEN_BUILD_ROOT:-${TMPDIR:-/tmp}/mednafen-firestaff-irq2-trace}
prefix="$build_root/install"

if [ ! -f "$source_root/src/drivers/debugger.cpp" ] ||
   [ ! -f "$repo/scripts/mednafen_1.32.1_theron_irq2_trace.patch" ]; then
    printf 'FAIL: expected Mednafen 1.32.1 source tree and Firestaff patch\n' >&2
    exit 1
fi
if [[ -n "$sdl2_prefix" ]]; then
    if [[ ! -f "$sdl2_prefix/lib/pkgconfig/sdl2.pc" ]]; then
        printf 'FAIL: FIRESTAFF_MEDNAFEN_SDL2_PREFIX must contain lib/pkgconfig/sdl2.pc\n' >&2
        exit 1
    fi
    export PKG_CONFIG_PATH="$sdl2_prefix/lib/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}"
fi

rm -rf "$build_root"
mkdir -p "$build_root"
cp -R "$source_root/." "$build_root/source"
# macOS's BSD patch rejects the large debugger hunk despite a clean 1.32.1
# source tree; git apply validates that hunk exactly. The smaller legacy
# patches intentionally retain their original BSD-patch format.
git -C "$build_root/source" apply --whitespace=nowarn \
    "$repo/scripts/mednafen_1.32.1_theron_irq2_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_pcecd_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_input_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_pcecd_state_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_input_state_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_host_input_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_cd_transfer_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_cd_caller_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_post_stage2_execution_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_post_stage2_e009_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_post_stage2_e00f_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_later_raw_sector_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_later_fifo_generation_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_generation7_fifo_ram_receipt.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_main_ram_loader_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_main_ram_e009_dispatch_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_main_ram_e009_fifo_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_g4_main_ram_read_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_main_ram_e009_owner_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_main_ram_loader_write_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_main_ram_control_window_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_main_ram_game_window_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_fifo_origin_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_later_generation_filter.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_all_generation_origin_ram_receipt.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_game_owned_origin_ram_receipt.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_parameter_window_trace.patch"

# The released Mednafen tree carries generated Makefile.in files. Copying it
# into a fresh trace root can make make try to regenerate them, which would
# require the historical automake-1.16 toolchain. Keep the shipped generated
# inputs authoritative for this instrumented build.
find "$build_root/source" -name Makefile.in -exec touch {} +

cd "$build_root/source"
# The Firestaff hook reads PCE registers through Mednafen's debugger API.
# Enabling the legacy PCECD_DEBUG printf path breaks current 1.32.1 builds
# because that path does not include the HuCPU declaration.
CXXFLAGS="${CXXFLAGS:-}" ./configure --prefix="$prefix" --disable-apple2 --disable-gb --disable-gba \
    --disable-lynx --disable-md --disable-nes --disable-ngp --disable-pce-fast \
    --disable-pcfx --disable-psx --disable-sasplay --disable-sms --disable-snes \
    --disable-snes-faust --disable-ss --disable-ssfplay --disable-vb --disable-wswan \
    --without-libflac
make -j"$(sysctl -n hw.ncpu)"
make install
"$repo/scripts/verify_theron_mednafen_sdl2_runtime.sh" "$prefix/bin/mednafen"
printf '%s\n' "$prefix/bin/mednafen"
