#!/usr/bin/env bash
# Build an external Mednafen 1.32.1 Saturn producer with Firestaff's
# read-only runtime witness hook. The script never copies BIOS, discs, or
# traces; those remain operator-owned inputs to the launcher.
set -euo pipefail

usage() {
  echo "usage: $0 --build-dir DIR --prefix DIR" >&2
}

build_dir=
prefix=
while (($#)); do
  case "$1" in
    --build-dir|--prefix)
      (($# >= 2)) || { usage; exit 2; }
      key=${1#--}; key=${key//-/_}; printf -v "$key" '%s' "$2"; shift 2 ;;
    *) usage; exit 2 ;;
  esac
done
: "${build_dir:?}" "${prefix:?}"

repo_root=$(cd "$(dirname "$0")/.." && pwd)
archive="$build_dir/mednafen-1.32.1.tar.xz"
source_dir="$build_dir/mednafen-1.32.1"
mkdir -p "$build_dir" "$prefix"
export TMPDIR="$build_dir/tmp"
mkdir -p "$TMPDIR"
if [[ ! -f "$archive" ]]; then
  curl -L --fail --silent --show-error \
    https://mednafen.github.io/releases/files/mednafen-1.32.1.tar.xz \
    -o "$archive"
fi
if [[ ! -f "$source_dir/configure" ]]; then
  tar -xf "$archive" -C "$build_dir"
  if [[ ! -f "$source_dir/configure" && -f "$build_dir/mednafen/configure" ]]; then
    mv "$build_dir/mednafen" "$source_dir"
  fi
fi
marker="$source_dir/.firestaff-nexus-capture-patched"
patch_id='FIRESTAFF_NEXUS_SATURN_CAPTURE_PATCH_V4_FRAME_INPUT_LOOP'
if [[ ! -f "$marker" ]]; then
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_saturn_capture.patch"
  printf '%s\n' "$patch_id" > "$marker"
elif [[ "$(cat "$marker" 2>/dev/null)" != "$patch_id" ]]; then
  echo "ERROR: external Mednafen source has an older or unknown Firestaff patch; use a fresh build directory" >&2
  exit 2
fi
cd_read_marker="$source_dir/.firestaff-nexus-cdb-read-trace-patched"
cd_read_patch_id='FIRESTAFF_NEXUS_SATURN_CDB_READ_TRACE_V1'
if [[ ! -f "$cd_read_marker" ]]; then
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_cd_read_trace.patch"
  printf '%s\n' "$cd_read_patch_id" > "$cd_read_marker"
elif [[ "$(cat "$cd_read_marker" 2>/dev/null)" != "$cd_read_patch_id" ]]; then
  echo "ERROR: external Mednafen source has an older or unknown CDB-read patch" >&2
  exit 2
fi
source_trace_marker="$source_dir/.firestaff-nexus-sh2-source-trace-patched"
source_trace_patch_id='FIRESTAFF_NEXUS_SH2_SOURCE_TRACE_V1_COMPOSABLE'
if [[ ! -f "$source_trace_marker" ]]; then
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_sh2_source_trace.patch"
  printf '%s\n' "$source_trace_patch_id" > "$source_trace_marker"
elif [[ "$(cat "$source_trace_marker" 2>/dev/null)" != "$source_trace_patch_id" ]]; then
  echo "ERROR: external Mednafen source has an older or unknown SH-2 source-trace patch" >&2
  exit 2
fi
sh2_memory_peek_marker="$source_dir/.firestaff-nexus-sh2-memory-peek-support-patched"
sh2_memory_peek_patch_id='FIRESTAFF_NEXUS_SH2_MEMORY_PEEK_SUPPORT_V1_COMPOSABLE'
if [[ ! -f "$sh2_memory_peek_marker" ]]; then
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_sh2_memory_peek_support.patch"
  printf '%s\n' "$sh2_memory_peek_patch_id" > "$sh2_memory_peek_marker"
elif [[ "$(cat "$sh2_memory_peek_marker" 2>/dev/null)" != "$sh2_memory_peek_patch_id" ]]; then
  echo "ERROR: external Mednafen source has an older or unknown SH-2 memory-peek support patch; use a fresh build directory" >&2
  exit 2
fi
sh2_register_trace_marker="$source_dir/.firestaff-nexus-sh2-register-trace-support-patched"
sh2_register_trace_patch_id='FIRESTAFF_NEXUS_SH2_REGISTER_TRACE_SUPPORT_V1_COMPOSABLE'
if [[ ! -f "$sh2_register_trace_marker" ]]; then
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_sh2_register_trace_support.patch"
  printf '%s\n' "$sh2_register_trace_patch_id" > "$sh2_register_trace_marker"
elif [[ "$(cat "$sh2_register_trace_marker" 2>/dev/null)" != "$sh2_register_trace_patch_id" ]]; then
  echo "ERROR: external Mednafen source has an older or unknown SH-2 register support patch; use a fresh build directory" >&2
  exit 2
fi
vdp2_trace_marker="$source_dir/.firestaff-nexus-vdp2-write-trace-patched"
vdp2_trace_patch_id='FIRESTAFF_NEXUS_VDP2_WRITE_TRACE_V3_CODE'
if [[ ! -f "$vdp2_trace_marker" ]]; then
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_vdp2_write_trace.patch"
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_vdp2_vram_write_trace.patch"
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_vdp2_cram_write_trace.patch"
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_vdp2_regs_write_trace.patch"
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_vdp2_writer_code_trace.patch"
  printf '%s\n' "$vdp2_trace_patch_id" > "$vdp2_trace_marker"
elif [[ "$(cat "$vdp2_trace_marker" 2>/dev/null)" != "$vdp2_trace_patch_id" ]]; then
  echo "ERROR: external Mednafen source has an older or unknown VDP2-write trace patch; use a fresh build directory" >&2
  exit 2
fi
vdp1_trace_marker="$source_dir/.firestaff-nexus-vdp1-write-trace-patched"
vdp1_trace_patch_id='FIRESTAFF_NEXUS_VDP1_WRITE_TRACE_V4_CODE_TARGETED_SNAPSHOT'
if [[ ! -f "$vdp1_trace_marker" ]]; then
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_saturn_vdp1_pc_trace.patch"
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_vdp1_pc_code_trace.patch"
  printf '%s\n' "$vdp1_trace_patch_id" > "$vdp1_trace_marker"
elif [[ "$(cat "$vdp1_trace_marker" 2>/dev/null)" != "$vdp1_trace_patch_id" ]]; then
  echo "ERROR: external Mednafen source has an older or unknown VDP1-write trace patch; use a fresh build directory" >&2
  exit 2
fi
slev_sal_capture_marker="$source_dir/.firestaff-nexus-slev-sal-capture-patched"
slev_sal_capture_patch_id='FIRESTAFF_NEXUS_SLEV_SAL_CAPTURE_V1_OPAQUE_RAM_WRITE'
if [[ ! -f "$slev_sal_capture_marker" ]]; then
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_slev_sal_capture.patch"
  printf '%s\n' "$slev_sal_capture_patch_id" > "$slev_sal_capture_marker"
elif [[ "$(cat "$slev_sal_capture_marker" 2>/dev/null)" != "$slev_sal_capture_patch_id" ]]; then
  echo "ERROR: external Mednafen source has an older or unknown SLEV/SAL capture patch; use a fresh build directory" >&2
  exit 2
fi
profile_marker="$source_dir/.firestaff-nexus-saturn-only"
profile_id='FIRESTAFF_NEXUS_MEDNAFEN_PROFILE_V2_SATURN_ONLY'
if [[ ! -f "$profile_marker" || "$(cat "$profile_marker" 2>/dev/null)" != "$profile_id" ]]; then
  (cd "$source_dir" && ./configure --prefix="$prefix" \
    --enable-ss \
    --disable-apple2 --disable-gb --disable-gba --disable-lynx \
    --disable-md --disable-nes --disable-ngp --disable-pce \
    --disable-pce-fast --disable-pcfx --disable-psx --disable-sasplay \
    --disable-sms --disable-snes --disable-snes-faust --disable-ssfplay \
    --disable-vb --disable-wswan --disable-cjk-fonts \
    --disable-fancy-scalers --disable-debugger --disable-alsa --disable-jack)
  printf '%s\n' "$profile_id" > "$profile_marker"
fi
if [[ ! -x "$source_dir/src/mednafen" ]]; then
  make -C "$source_dir" -j"${FIRESTAFF_MEDNAFEN_JOBS:-2}"
fi
make -C "$source_dir" install
capture_bin="$prefix/bin/mednafen"
strings "$capture_bin" | grep -F \
  'FIRESTAFF_NEXUS_TRACE_OUTPUT' >/dev/null
strings "$capture_bin" | grep -F \
  'FIRESTAFF_NEXUS_SLEV_SAL_CAPTURE_OUTPUT' >/dev/null
printf 'instrumented_mednafen=%s\n' "$capture_bin"
printf 'source_patch=%s\n' "$repo_root/scripts/mednafen_1.32.1_nexus_saturn_capture.patch"
