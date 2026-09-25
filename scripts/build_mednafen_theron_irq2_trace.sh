#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -gt 1 ]; then
    printf 'usage: %s [MEDNAFEN_1.32.1_SOURCE]\n' "$0" >&2
    exit 2
fi

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
# The original source is licensed third-party material.  Require its location
# explicitly instead of silently assuming a shared temporary directory.
source_root=${1:-${FIRESTAFF_MEDNAFEN_SOURCE_ROOT:-}}
sdl2_prefix=${FIRESTAFF_MEDNAFEN_SDL2_PREFIX:-}
patch_only=${FIRESTAFF_MEDNAFEN_PATCH_ONLY:-0}
# An explicit build root keeps parallel local investigations from reusing an
# instrumented binary produced from a different patch revision.
build_root=${FIRESTAFF_MEDNAFEN_BUILD_ROOT:-"$repo/.codex-scratch/mednafen-firestaff-irq2-trace"}
prefix="$build_root/install"

if [[ -z "$source_root" ]]; then
    printf 'FAIL: provide MEDNAFEN_1.32.1_SOURCE or FIRESTAFF_MEDNAFEN_SOURCE_ROOT\n' >&2
    exit 2
fi
if [[ "$patch_only" != 0 && "$patch_only" != 1 ]]; then
    printf 'FAIL: FIRESTAFF_MEDNAFEN_PATCH_ONLY must be 0 or 1\n' >&2
    exit 2
fi
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

case "$build_root" in
    /|"$repo"|"$source_root")
        printf 'FAIL: FIRESTAFF_MEDNAFEN_BUILD_ROOT is not a safe dedicated build directory\n' >&2
        exit 2
        ;;
esac
if [[ -e "$build_root" ]]; then
    if [[ "${FIRESTAFF_MEDNAFEN_ALLOW_REBUILD:-0}" != 1 ]]; then
        printf 'FAIL: build root already exists; set FIRESTAFF_MEDNAFEN_ALLOW_REBUILD=1 to replace it\n' >&2
        exit 2
    fi
    rm -rf -- "$build_root"
fi
mkdir -p "$build_root"
cp -R "$source_root/." "$build_root/source"
# The supplied Mednafen release source is an exported source tree, not a Git
# checkout. git apply silently skips patches when their paths are absent from
# an index, so create one inside this disposable copy before applying hooks.
git -C "$build_root/source" init --quiet
git -C "$build_root/source" add --all --force
# macOS's BSD patch rejects the large debugger hunk despite a clean 1.32.1
# source tree; git apply validates that hunk exactly. The smaller trace
# patches retain their original BSD-patch format.
git -C "$build_root/source" apply --recount --whitespace=nowarn \
    "$repo/scripts/mednafen_1.32.1_theron_irq2_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_post_stage2_execution_trace.patch"
git -C "$build_root/source" apply --recount --whitespace=nowarn \
    "$repo/scripts/mednafen_1.32.1_theron_rng_consumer_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_pcecd_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_input_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_input_grab_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_pcecd_state_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_input_state_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_input_result_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_scripted_pce_input.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_host_input_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_cd_transfer_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_cd_transfer_owner_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_cd_caller_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_main_ram_loader_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_main_ram_e009_window_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_main_ram_e009_critical_trace.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_main_ram_e009_register_trace.patch"
adpcm_context_patch="$repo/scripts/mednafen_1.32.1_theron_adpcm_fifo_ram_trace_context.patch"
adpcm_context_rendered="$build_root/theron-adpcm-fifo-ram-trace.rendered.patch"
# The checked-in patch uses a visible token for unified-diff blank context so
# the repository's trailing-whitespace gate cannot erase the context byte.
sed 's/^FIRESTAFF_PATCH_BLANK_CONTEXT$/ /' "$adpcm_context_patch" > "$adpcm_context_rendered"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$adpcm_context_rendered"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_adpcm_playback_trace.patch"
main_ram_consumer_read_patch="$repo/scripts/mednafen_1.32.1_theron_main_ram_consumer_read_trace.patch"
main_ram_consumer_read_rendered="$build_root/theron-main-ram-consumer-read.rendered.patch"
sed 's/^FIRESTAFF_PATCH_BLANK_CONTEXT$/ /' "$main_ram_consumer_read_patch" \
    > "$main_ram_consumer_read_rendered"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$main_ram_consumer_read_rendered"
patch -d "$build_root/source" -p1 --batch --forward \
    < <(sed 's/^ FIRESTAFF_PATCH_BLANK_CONTEXT$/ /' \
        "$repo/scripts/mednafen_1.32.1_theron_main_ram_consumer_write_trace.patch")
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_e009_destination_consumer_trace.patch"
git -C "$build_root/source" apply --recount --whitespace=nowarn \
    "$repo/scripts/mednafen_1.32.1_theron_e009_destination_consumer_read.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_e009_consumer_code_snapshot.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_fifo_origin_main_ram_consumer_v2.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_adpcm_fifo_direct_read_origin_fix.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_state_autoload.patch"
git -C "$build_root/source" apply --recount --whitespace=nowarn \
    "$repo/scripts/mednafen_1.32.1_theron_vram_vce_snapshot.patch"
loader_write_patch="$repo/scripts/mednafen_1.32.1_theron_main_ram_loader_write_trace_v3.patch"
loader_write_rendered="$build_root/theron-main-ram-loader-write.rendered.patch"
sed \
    -e 's/^FIRESTAFF_PATCH_BLANK_CONTEXT$/ /' \
    -e $'s/^FIRESTAFF_PATCH_TAB_CONTEXT/ \t/' \
    "$loader_write_patch" > "$loader_write_rendered"
git -C "$build_root/source" apply --recount --whitespace=nowarn \
    "$loader_write_rendered"
ram_provenance_patch="$repo/scripts/mednafen_1.32.1_theron_ram_provenance_trace.patch"
ram_provenance_rendered="$build_root/theron-ram-provenance.rendered.patch"
sed \
    -e 's/^FIRESTAFF_PATCH_BLANK_CONTEXT$/ /' \
    -e $'s/^FIRESTAFF_PATCH_TAB_CONTEXT/ \t/' \
    "$ram_provenance_patch" \
    > "$ram_provenance_rendered"
git -C "$build_root/source" apply --recount --whitespace=nowarn \
    "$ram_provenance_rendered"
git -C "$build_root/source" apply --recount --ignore-space-change \
    --whitespace=nowarn \
    "$repo/scripts/mednafen_1.32.1_theron_single_logical_write_fix.patch"
vdc_io_patch="$repo/scripts/mednafen_1.32.1_theron_vdc_io_trace.patch"
vdc_io_rendered="$build_root/theron-vdc-io-trace.rendered.patch"
sed \
    -e 's/^FIRESTAFF_PATCH_BLANK_CONTEXT$/ /' \
    -e $'s/^FIRESTAFF_PATCH_VDC_WRITE_CONTEXT$/ \t       vce->WriteVDC(A \\& 0x80001FFF, V);/' \
    -e $'s/^FIRESTAFF_PATCH_VDC_BREAK_CONTEXT$/ \t       break;/' \
    "$vdc_io_patch" > "$vdc_io_rendered"
patch -d "$build_root/source" -p1 --batch --forward < "$vdc_io_rendered"
git -C "$build_root/source" apply --recount --whitespace=nowarn \
    "$repo/scripts/mednafen_1.32.1_theron_scsi_generation_vdc_trace.patch"
git -C "$build_root/source" apply \
    "$repo/scripts/mednafen_1.32.1_theron_file_select_vdc_snapshot.patch"
patch -d "$build_root/source" -p1 --batch --forward \
    < "$repo/scripts/mednafen_1.32.1_theron_file_select_scroll_driver_trace.patch"
git -C "$build_root/source" apply --recount --whitespace=nowarn \
    "$repo/scripts/mednafen_1.32.1_theron_game_main_ram_e009_destination_receipt.patch"
git -C "$build_root/source" apply --recount --whitespace=nowarn \
    "$repo/scripts/mednafen_1.32.1_theron_post_dungeon_ordinal_research.patch"
git -C "$build_root/source" apply --recount --whitespace=nowarn \
    "$repo/scripts/mednafen_1.32.1_theron_title_wait_input_research.patch"
git -C "$build_root/source" apply --recount --whitespace=nowarn \
    "$repo/scripts/mednafen_1.32.1_theron_drator_menu_route_research.patch"
git -C "$build_root/source" apply --recount --whitespace=nowarn \
    "$repo/scripts/mednafen_1.32.1_theron_save_manager_code_dump.patch"
git -C "$build_root/source" apply --recount --whitespace=nowarn \
    "$repo/scripts/mednafen_1.32.1_theron_selected_record_consumer_trace.patch"

if [[ "$patch_only" == 1 ]]; then
    # Tests use the exact production patch order without paying for a rebuild.
    printf 'PASS: complete Theron Mednafen patch set applied to isolated source copy\n'
    exit 0
fi

# The FIFO-origin extension is capture-only. It carries raw LBA/offset/FIFO
# provenance into the CD-transfer receipt; it does not assign level, object,
# tile, palette, or viewport semantics and does not authorize runtime drawing.
# Older G4/FIFO-origin extensions target a different debugger hook and remain
# research-only.

# The released Mednafen tree carries generated Makefile.in files. Copying it
# into a fresh trace root can make make try to regenerate them, which would
# require the historical automake-1.16 toolchain. Keep the shipped generated
# inputs authoritative for this instrumented build.
find "$build_root/source" -name Makefile.in -exec touch {} +

cd "$build_root/source"
# The Firestaff hook reads PCE registers through Mednafen's debugger API.
# Enabling the legacy PCECD_DEBUG printf path breaks current 1.32.1 builds
# because that path does not include the HuCPU declaration.
if [[ "$(uname -s)" == Darwin && -n "$sdl2_prefix" ]]; then
    export LDFLAGS="${LDFLAGS:-} -Wl,-rpath,$sdl2_prefix/lib"
fi
# The PCE interpreter's unoptimized frame exceeds the default macOS emulator
# thread stack and faults in the stack probe on Apple Silicon. Keep an
# explicit caller-provided CXXFLAGS, but make ordinary capture builds usable.
CXXFLAGS="${CXXFLAGS:--O2}" ./configure --prefix="$prefix" --disable-apple2 --disable-gb --disable-gba \
    --disable-lynx --disable-md --disable-nes --disable-ngp --disable-pce-fast \
    --disable-pcfx --disable-psx --disable-sasplay --disable-sms --disable-snes \
    --disable-snes-faust --disable-ss --disable-ssfplay --disable-vb --disable-wswan \
    --without-libflac
if command -v getconf >/dev/null 2>&1; then
    build_jobs=$(getconf _NPROCESSORS_ONLN 2>/dev/null || true)
fi
if [[ ! ${build_jobs:-} =~ ^[1-9][0-9]*$ ]] && command -v sysctl >/dev/null 2>&1; then
    build_jobs=$(sysctl -n hw.ncpu 2>/dev/null || true)
fi
if [[ -n ${FIRESTAFF_MEDNAFEN_BUILD_JOBS:-} ]]; then
    if [[ ! $FIRESTAFF_MEDNAFEN_BUILD_JOBS =~ ^[1-9][0-9]*$ ]]; then
        printf 'FAIL: FIRESTAFF_MEDNAFEN_BUILD_JOBS must be a positive integer\n' >&2
        exit 2
    fi
    build_jobs=$FIRESTAFF_MEDNAFEN_BUILD_JOBS
fi
build_jobs=${build_jobs:-1}
make -j"$build_jobs"
make install
"$repo/scripts/verify_theron_mednafen_sdl2_runtime.sh" "$prefix/bin/mednafen"
printf '%s\n' "$prefix/bin/mednafen"
