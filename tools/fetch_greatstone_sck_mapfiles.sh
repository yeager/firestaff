#!/usr/bin/env sh
set -eu

url="${FIRESTAFF_GREATSTONE_SCK_URL:-http://greatstone.free.fr/dm/download/sck@1.5.1@20110502.0.zip}"
out_dir="${1:-${HOME}/.cache/firestaff/greatstone-sck-mapfiles}"
tmp_dir="${TMPDIR:-/tmp}/firestaff-greatstone-sck-mapfiles.$$"

cleanup() {
    rm -rf "$tmp_dir"
}
trap cleanup EXIT INT HUP TERM

mkdir -p "$tmp_dir" "$out_dir"

curl -L --fail --retry 3 --connect-timeout 15 --max-time 120 \
    -o "$tmp_dir/sck.zip" "$url"

unzip -q "$tmp_dir/sck.zip" 'sck/sck.jar' -d "$tmp_dir"
unzip -q "$tmp_dir/sck/sck.jar" 'db/map/*.map' 'db/map/_mapping.xml' -d "$out_dir"

{
    printf 'source_url=%s\n' "$url"
    printf 'fetched_at_utc=%s\n' "$(date -u '+%Y-%m-%dT%H:%M:%SZ')"
    printf 'map_count=%s\n' "$(find "$out_dir/db/map" -type f -name '*.map' | wc -l | tr -d ' ')"
    printf 'mapping=%s\n' "$out_dir/db/map/_mapping.xml"
} > "$out_dir/SOURCE.txt"

printf 'Greatstone SCK mapfiles written to %s\n' "$out_dir"
