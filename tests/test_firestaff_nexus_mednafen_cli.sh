#!/bin/sh
# Exercise Firestaff's explicit authentic-Nexus emulator handoff without
# starting a window or needing proprietary media in the test tree.

set -eu

firestaff_bin=${FIRESTAFF_BIN:?FIRESTAFF_BIN must name the firestaff executable}
test_dir=$(mktemp -d "${TMPDIR:-/tmp}/firestaff-nexus-mednafen.XXXXXX")
trap 'rm -rf "$test_dir"' EXIT HUP INT TERM

mkdir -p "$test_dir/data/nexus"
cue="$test_dir/data/nexus/Dungeon Master Nexus (English).cue"
bios="$test_dir/Sega Saturn BIOS (J).bin"
fake="$test_dir/fake-mednafen"
log="$test_dir/arguments.log"

printf '%s\n' 'FILE "Dungeon Master Nexus (English).iso" BINARY' \
    '  TRACK 01 MODE1/2048' '    INDEX 01 00:00:00' > "$cue"
printf '%s\n' 'test bios only' > "$bios"
printf '%s\n' '#!/bin/sh' 'printf "%s\\n" "$@" > "$FIRESTAFF_NEXUS_FAKE_LOG"' > "$fake"
chmod +x "$fake"

output=$(FIRESTAFF_NEXUS_FAKE_LOG="$log" "$firestaff_bin" \
    --game nexus --data-dir "$test_dir/data" --nexus-mednafen "$fake" \
    --nexus-bios "$bios" 2>&1) || {
    printf '%s\n' "$output" >&2
    exit 1
}
case "$output" in
    *'FIRESTAFF NEXUS EXTERNAL LAUNCH:'*) ;;
    *) printf '%s\n' 'fail: missing external launch receipt' >&2; exit 1 ;;
esac
expected=$(printf '%s\n%s\n%s' '-ss.bios_jp' "$bios" "$cue")
actual=$(cat "$log")
[ "$actual" = "$expected" ] || {
    printf '%s\n' 'fail: Mednafen arguments differ from expected receipt' >&2
    exit 1
}

if FIRESTAFF_NEXUS_FAKE_LOG="$log" "$firestaff_bin" \
    --game dm1 --nexus-mednafen "$fake" >/dev/null 2>&1; then
    printf '%s\n' 'fail: non-Nexus game was accepted by --nexus-mednafen' >&2
    exit 1
fi
if FIRESTAFF_NEXUS_FAKE_LOG="$log" "$firestaff_bin" \
    --game nexus --boot-probe --nexus-mednafen "$fake" >/dev/null 2>&1; then
    printf '%s\n' 'fail: boot probe was accepted with external Nexus launch' >&2
    exit 1
fi

printf '%s\n' 'test_firestaff_nexus_mednafen_cli: PASS'
