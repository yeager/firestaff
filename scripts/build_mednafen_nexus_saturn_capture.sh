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
patch_id='FIRESTAFF_NEXUS_SATURN_CAPTURE_PATCH_V5_POST_RENDER_FRAME'
if [[ ! -f "$marker" ]]; then
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_saturn_capture.patch"
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_capture_post_render.patch"
  printf '%s\n' "$patch_id" > "$marker"
elif [[ "$(cat "$marker" 2>/dev/null)" != "$patch_id" ]]; then
  echo "ERROR: external Mednafen source has an older or unknown Firestaff patch; use a fresh build directory" >&2
  exit 2
fi
input_sequence_marker="$source_dir/.firestaff-nexus-input-sequence-patched"
input_sequence_patch_id='FIRESTAFF_NEXUS_INPUT_SEQUENCE_V1_13BIT'
if [[ ! -f "$input_sequence_marker" ]]; then
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_input_sequence.patch"
  printf '%s\n' "$input_sequence_patch_id" > "$input_sequence_marker"
elif [[ "$(cat "$input_sequence_marker" 2>/dev/null)" != "$input_sequence_patch_id" ]]; then
  echo "ERROR: external Mednafen source has an older or unknown input-sequence patch; use a fresh build directory" >&2
  exit 2
fi
input_trace_marker="$source_dir/.firestaff-nexus-input-trace-patched"
input_trace_patch_id='FIRESTAFF_NEXUS_INPUT_TRACE_V1'
if [[ ! -f "$input_trace_marker" ]]; then
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_input_trace.patch"
  printf '%s\n' "$input_trace_patch_id" > "$input_trace_marker"
elif [[ "$(cat "$input_trace_marker" 2>/dev/null)" != "$input_trace_patch_id" ]]; then
  echo "ERROR: external Mednafen source has an older or unknown input trace patch; use a fresh build directory" >&2
  exit 2
fi
cd_read_marker="$source_dir/.firestaff-nexus-cdb-read-trace-patched"
cd_read_patch_id='FIRESTAFF_NEXUS_SATURN_CDB_READ_TRACE_V2_LBA_FILTER'
if [[ ! -f "$cd_read_marker" ]]; then
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_cd_read_trace.patch"
  printf '%s\n' "$cd_read_patch_id" > "$cd_read_marker"
elif [[ "$(cat "$cd_read_marker" 2>/dev/null)" != "$cd_read_patch_id" ]]; then
  echo "ERROR: external Mednafen source has an older or unknown CDB-read patch" >&2
  exit 2
fi
source_trace_marker="$source_dir/.firestaff-nexus-sh2-source-trace-patched"
source_trace_patch_id='FIRESTAFF_NEXUS_SH2_SOURCE_TRACE_V6_CDB_FIFO_LBA_FILTER'
if [[ ! -f "$source_trace_marker" ]]; then
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_sh2_source_trace.patch"
  printf '%s\n' "$source_trace_patch_id" > "$source_trace_marker"
elif [[ "$(cat "$source_trace_marker" 2>/dev/null)" != "$source_trace_patch_id" ]]; then
  echo "ERROR: external Mednafen source has an older or unknown SH-2 source-trace patch" >&2
  exit 2
fi
sh2_ram_read_marker="$source_dir/.firestaff-nexus-sh2-ram-read-trace-patched"
sh2_ram_read_patch_id='FIRESTAFF_NEXUS_SH2_RAM_READ_TRACE_V1_PC_FILTER'
if [[ ! -f "$sh2_ram_read_marker" ]]; then
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_sh2_ram_read_trace.patch"
  printf '%s\n' "$sh2_ram_read_patch_id" > "$sh2_ram_read_marker"
elif [[ "$(cat "$sh2_ram_read_marker" 2>/dev/null)" != "$sh2_ram_read_patch_id" ]]; then
  echo "ERROR: external Mednafen source has an older or unknown SH-2 RAM-read trace patch; use a fresh build directory" >&2
  exit 2
fi
sh2_cached_ram_read_marker="$source_dir/.firestaff-nexus-sh2-cached-ram-read-trace-patched"
sh2_cached_ram_read_patch_id='FIRESTAFF_NEXUS_SH2_CACHED_RAM_READ_TRACE_V1_PC_FILTER'
if [[ ! -f "$sh2_cached_ram_read_marker" ]]; then
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_sh2_cached_ram_read_trace.patch"
  printf '%s\n' "$sh2_cached_ram_read_patch_id" > "$sh2_cached_ram_read_marker"
elif [[ "$(cat "$sh2_cached_ram_read_marker" 2>/dev/null)" != "$sh2_cached_ram_read_patch_id" ]]; then
  echo "ERROR: external Mednafen source has an older or unknown SH-2 cached-RAM-read trace patch; use a fresh build directory" >&2
  exit 2
fi
sh2_cached_ram_read_filter_marker="$source_dir/.firestaff-nexus-sh2-cached-ram-read-title-pc-filter-patched"
sh2_cached_ram_read_filter_patch_id='FIRESTAFF_NEXUS_SH2_CACHED_RAM_READ_TITLE_PC_FILTER_V1'
if [[ ! -f "$sh2_cached_ram_read_filter_marker" ]]; then
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_sh2_cached_ram_read_title_pc_filter.patch"
  printf '%s\n' "$sh2_cached_ram_read_filter_patch_id" > "$sh2_cached_ram_read_filter_marker"
elif [[ "$(cat "$sh2_cached_ram_read_filter_marker" 2>/dev/null)" != "$sh2_cached_ram_read_filter_patch_id" ]]; then
  echo "ERROR: external Mednafen source has an older or unknown SH-2 cached-RAM-read title-PC filter patch; use a fresh build directory" >&2
  exit 2
fi
sh2_cache_bypass_read_marker="$source_dir/.firestaff-nexus-sh2-cache-bypass-title-read-trace-patched"
sh2_cache_bypass_read_patch_id='FIRESTAFF_NEXUS_SH2_CACHE_BYPASS_TITLE_READ_TRACE_V1'
if [[ ! -f "$sh2_cache_bypass_read_marker" ]]; then
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_sh2_cache_bypass_title_read_trace.patch"
  printf '%s\n' "$sh2_cache_bypass_read_patch_id" > "$sh2_cache_bypass_read_marker"
elif [[ "$(cat "$sh2_cache_bypass_read_marker" 2>/dev/null)" != "$sh2_cache_bypass_read_patch_id" ]]; then
  echo "ERROR: external Mednafen source has an older or unknown SH-2 cache-bypass title-read trace patch; use a fresh build directory" >&2
  exit 2
fi
sh2_cache_bypass_address_marker="$source_dir/.firestaff-nexus-sh2-cache-bypass-title-address-filter-patched"
sh2_cache_bypass_address_patch_id='FIRESTAFF_NEXUS_SH2_CACHE_BYPASS_TITLE_ADDRESS_FILTER_V1'
if [[ ! -f "$sh2_cache_bypass_address_marker" ]]; then
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_sh2_cache_bypass_title_address_filter.patch"
  printf '%s\n' "$sh2_cache_bypass_address_patch_id" > "$sh2_cache_bypass_address_marker"
elif [[ "$(cat "$sh2_cache_bypass_address_marker" 2>/dev/null)" != "$sh2_cache_bypass_address_patch_id" ]]; then
  echo "ERROR: external Mednafen source has an older or unknown SH-2 cache-bypass title-address-filter patch; use a fresh build directory" >&2
  exit 2
fi
sh2_instruction_byte_read_marker="$source_dir/.firestaff-nexus-sh2-instruction-byte-read-trace-patched"
sh2_instruction_byte_read_patch_id='FIRESTAFF_NEXUS_SH2_INSTRUCTION_BYTE_READ_TRACE_V1'
if [[ ! -f "$sh2_instruction_byte_read_marker" ]]; then
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_sh2_instruction_byte_read_trace.patch"
  printf '%s\n' "$sh2_instruction_byte_read_patch_id" > "$sh2_instruction_byte_read_marker"
elif [[ "$(cat "$sh2_instruction_byte_read_marker" 2>/dev/null)" != "$sh2_instruction_byte_read_patch_id" ]]; then
  echo "ERROR: external Mednafen source has an older or unknown SH-2 instruction byte-read trace patch; use a fresh build directory" >&2
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
sh2_ram_write_marker="$source_dir/.firestaff-nexus-sh2-ram-write-trace-patched"
sh2_ram_write_patch_id='FIRESTAFF_NEXUS_SH2_RAM_WRITE_TRACE_V1_DMA_PROVENANCE'
if [[ "${FIRESTAFF_NEXUS_ENABLE_SH2_RAM_TRACE:-0}" = 1 ]]; then
  if [[ ! -f "$sh2_ram_write_marker" ]]; then
    patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_sh2_ram_write_trace.patch"
    printf '%s\n' "$sh2_ram_write_patch_id" > "$sh2_ram_write_marker"
  elif [[ "$(cat "$sh2_ram_write_marker" 2>/dev/null)" != "$sh2_ram_write_patch_id" ]]; then
    echo "ERROR: external Mednafen source has an older or unknown SH-2 RAM-write trace patch; use a fresh build directory" >&2
    exit 2
  fi
fi
sh2_memory_snapshot_marker="$source_dir/.firestaff-nexus-sh2-memory-snapshot-patched"
sh2_memory_snapshot_patch_id='FIRESTAFF_NEXUS_SH2_MEMORY_SNAPSHOT_V1'
if [[ "${FIRESTAFF_NEXUS_ENABLE_SH2_MEMORY_SNAPSHOT:-0}" = 1 ]]; then
  if [[ ! -f "$sh2_memory_snapshot_marker" ]]; then
    patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_sh2_memory_snapshot.patch"
    patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_sh2_memory_snapshot_hook.patch"
    printf '%s\n' "$sh2_memory_snapshot_patch_id" > "$sh2_memory_snapshot_marker"
  elif [[ "$(cat "$sh2_memory_snapshot_marker" 2>/dev/null)" != "$sh2_memory_snapshot_patch_id" ]]; then
    echo "ERROR: external Mednafen source has an older or unknown SH-2 memory snapshot patch; use a fresh build directory" >&2
    exit 2
  fi
fi
vdp2_trace_marker="$source_dir/.firestaff-nexus-vdp2-write-trace-patched"
vdp2_trace_patch_id='FIRESTAFF_NEXUS_VDP2_WRITE_TRACE_V12_TITLE_R5_PREIMAGE'
if [[ ! -f "$vdp2_trace_marker" ]]; then
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_vdp2_write_trace.patch"
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_vdp2_vram_write_trace.patch"
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_vdp2_cram_write_trace.patch"
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_vdp2_regs_write_trace.patch"
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_vdp2_writer_code_trace.patch"
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_vdp2_writer_register_trace.patch"
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_vdp2_writer_source_bytes_trace.patch"
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_vdp2_writer_source_r5_trace.patch"
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_vdp2_writer_source_r8_trace.patch"
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_vdp2_writer_frame_trace.patch"
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_vdp2_post_write_snapshot.patch"
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_vdp2_writer_register_pc_filter.patch"
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_vdp2_write_pc_filter.patch"
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_vdp2_emulation_frame_filter.patch"
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_vdp2_write_emulation_frame_filter.patch"
  printf '%s\n' "$vdp2_trace_patch_id" > "$vdp2_trace_marker"
elif [[ "$(cat "$vdp2_trace_marker" 2>/dev/null)" != "$vdp2_trace_patch_id" ]]; then
  echo "ERROR: external Mednafen source has an older or unknown VDP2-write trace patch; use a fresh build directory" >&2
  exit 2
fi
render_frame_marker="$source_dir/.firestaff-nexus-render-frame-trace-patched"
render_frame_patch_id='FIRESTAFF_NEXUS_RENDER_FRAME_TRACE_V1_PPM'
if [[ ! -f "$render_frame_marker" ]]; then
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_render_frame_trace.patch"
  printf '%s\n' "$render_frame_patch_id" > "$render_frame_marker"
elif [[ "$(cat "$render_frame_marker" 2>/dev/null)" != "$render_frame_patch_id" ]]; then
  echo "ERROR: external Mednafen source has an older or unknown render-frame trace patch; use a fresh build directory" >&2
  exit 2
fi
vdp1_trace_marker="$source_dir/.firestaff-nexus-vdp1-write-trace-patched"
vdp1_trace_patch_id='FIRESTAFF_NEXUS_VDP1_WRITE_TRACE_V6_REGISTER_SOURCE_WITNESS'
if [[ ! -f "$vdp1_trace_marker" ]]; then
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_saturn_vdp1_pc_trace.patch"
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_vdp1_pc_code_trace.patch"
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_vdp1_frame_trace.patch"
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_vdp1_writer_register_trace.patch"
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
scsp_trace_marker="$source_dir/.firestaff-nexus-slev-scsp-trace-patched"
scsp_trace_patch_id='FIRESTAFF_NEXUS_SLEV_SCSP_TRACE_V2_MAIN_AND_SOUND_CPU_SESSION'
if [[ ! -f "$scsp_trace_marker" ]]; then
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_slev_scsp_trace.patch"
  printf '%s\n' "$scsp_trace_patch_id" > "$scsp_trace_marker"
elif [[ "$(cat "$scsp_trace_marker" 2>/dev/null)" != "$scsp_trace_patch_id" ]]; then
  echo "ERROR: external Mednafen source has an older or unknown SCSP trace patch; use a fresh build directory" >&2
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
strings "$capture_bin" | grep -F \
  'FIRESTAFF_NEXUS_TRACE_SCSP_WRITES' >/dev/null
strings "$capture_bin" | grep -F \
  'FIRESTAFF_NEXUS_TRACE_MAIN_SCSP_WRITES' >/dev/null
strings "$capture_bin" | grep -F \
  'FIRESTAFF_NEXUS_TRACE_SESSION' >/dev/null
strings "$capture_bin" | grep -F \
  'FIRESTAFF_NEXUS_TRACE_PRESS_SEQUENCE' >/dev/null
strings "$capture_bin" | grep -F \
  'FIRESTAFF_NEXUS_TRACE_RENDER_FRAMES' >/dev/null
printf 'instrumented_mednafen=%s\n' "$capture_bin"
printf 'source_patch=%s\n' "$repo_root/scripts/mednafen_1.32.1_nexus_saturn_capture.patch"
