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
if [[ ! -f "$marker" ]]; then
  patch -d "$source_dir" -p0 < "$repo_root/scripts/mednafen_1.32.1_nexus_saturn_capture.patch"
  touch "$marker"
fi
if [[ ! -x "$source_dir/mednafen" ]]; then
  (cd "$source_dir" && ./configure --prefix="$prefix")
  make -C "$source_dir" -j"${FIRESTAFF_MEDNAFEN_JOBS:-2}"
fi
make -C "$source_dir" install
capture_bin="$prefix/bin/mednafen"
strings "$capture_bin" | grep -F \
  'FIRESTAFF_NEXUS_TRACE_OUTPUT' >/dev/null
printf 'instrumented_mednafen=%s\n' "$capture_bin"
printf 'source_patch=%s\n' "$repo_root/scripts/mednafen_1.32.1_nexus_saturn_capture.patch"
