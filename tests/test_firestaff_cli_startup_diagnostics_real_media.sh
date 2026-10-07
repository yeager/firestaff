#!/bin/sh
set -eu

firestaff_cli=${1:?Firestaff executable is required}
mac_archive=${FIRESTAFF_DM2_MAC_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_Mac_EN (1).zip"}
towns_archive=${FIRESTAFF_DM2_FMTOWNS_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip"}
dm1_towns_archive=${FIRESTAFF_DM1_FMTOWNS_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_FM-Towns_JA-EN.zip"}
dm1_pc_archive=${FIRESTAFF_DM1_PC_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_DOS_EN.zip"}
dm1_pc98_archive=${FIRESTAFF_DM1_PC98_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_PC-98_EN.zip"}
csb_towns_archive=${FIRESTAFF_CSB_FMTOWNS_ARCHIVE:-"$HOME/.firestaff/data/csb/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip"}
csb_towns_loose_root=${FIRESTAFF_CSB_FMTOWNS_LOOSE_ROOT:-"$HOME/.firestaff/data/csb/fmtowns_iso"}

if [ ! -x "$firestaff_cli" ] || [ ! -f "$mac_archive" ] ||
   [ ! -f "$towns_archive" ]; then
    echo "SKIP: original DM2 Macintosh and FM Towns archives are required"
    exit 77
fi

mac_debug=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --game dm2 --platform auto --data-dir "$mac_archive" \
    --debug --boot-probe --boot-probe-frames 0 2>&1) || {
    printf '%s\n' "$mac_debug" >&2
    exit 1
}
case "$mac_debug" in
    *"task=search root path="*"candidate-file=GRAPHICS.DAT"*"launch phase=game-handoff mode=direct game=dm2 platform=Macintosh edition=mac-en-retail source="*) ;;
    *) echo "FAIL: Mac debug trace omitted searched source or selected edition" >&2; exit 1 ;;
esac

towns_verbose=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --game dm2 --platform auto --data-dir "$towns_archive" \
    --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
    printf '%s\n' "$towns_verbose" >&2
    exit 1
}
case "$towns_verbose" in
    *"selected game=dm2 platform=FM Towns edition=fmtowns-ja source="*) ;;
    *) echo "FAIL: FM Towns verbose trace omitted selected edition" >&2; exit 1 ;;
esac
case "$towns_verbose" in
    *"candidate-file="*"edition-result="*) ;;
    *) echo "FAIL: verbose omitted catalogued candidate filenames" >&2; exit 1 ;;
esac

# --debug must expose every original FM Towns TWANIM frame on the ordinary
# no-platform direct CLI route, including source completion time. Boot probes
# fast-forward source waits and cannot verify this timing contract.
towns_debug=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --game dm2 --data-dir "$towns_archive" --debug --duration 40000 2>&1) || {
    printf '%s\n' "$towns_debug" >&2
    exit 1
}
printf '%s\n' "$towns_debug" | python3 -c '
import re
import sys

trace = sys.stdin.read()
frames = set()
finished = []
rejected = False
pattern = re.compile(
    r"startup-frame game=dm2 elapsed-ms=(\d+).*?"
    r"dm2-fmtowns-title=\{bound:(\d+) finished:(\d+) "
    r"rejected:(\d+) swoosh:(\d+) frame:(\d+)/(\d+) ticks-remaining:\d+\}"
)
for match in pattern.finditer(trace):
    elapsed, bound, done, reject, swoosh, frame, maximum = map(int, match.groups())
    rejected |= reject != 0
    if bound and not done and not swoosh and maximum == 225 and 0 < frame < 225:
        frames.add(frame)
    if done and not reject:
        finished.append((int(elapsed), int(frame), int(maximum)))
if (frames != set(range(1, 225)) or not finished or
        finished[-1][1:] != (225, 0) or
        not 24000 <= finished[-1][0] < 40000 or rejected or
        "phase=dm2-startup-menu" not in trace):
    raise SystemExit(
        "FAIL: DM2 --debug did not record every timed TWANIM frame and source completion timing"
    )
print(
    f"PASS: DM2 --debug recorded all timed TWANIM frames and finished in "
    f"{finished[-1][0]} ms"
)
' || {
    printf '%s\n' "$towns_debug" >&2
    exit 1
}

combined_root=$(mktemp -d "${PWD}/.firestaff-cli-diagnostics.XXXXXX")
csb_mixed_root=
dm1_combined_root=
default_home=
trap 'rm -rf "$combined_root" "$csb_mixed_root" "$dm1_combined_root" "$default_home"' EXIT

# Exercise the real default path layout without --data-dir and without
# inheriting a developer's saved config. Stage only links to the original
# archives already selected above; no game data is copied or synthesized.
default_home=$(mktemp -d "${PWD}/.firestaff-default-data-home.XXXXXX")
default_data_root="$default_home/.firestaff/data"
link_default_game_archive() {
    game_id=$1
    archive_path=$2
    if [ -f "$archive_path" ]; then
        mkdir -p "$default_data_root/$game_id"
        ln -s "$archive_path" \
            "$default_data_root/$game_id/$(basename "$archive_path")"
    fi
}
link_default_game_archive dm2 "$mac_archive"
link_default_game_archive dm2 "$towns_archive"
link_default_game_archive dm1 "$dm1_towns_archive"
link_default_game_archive csb "$csb_towns_archive"

for game_id in dm1 csb dm2; do
    if [ -d "$default_data_root/$game_id" ]; then
        default_output=$(HOME="$default_home" \
            XDG_CONFIG_HOME="$default_home/.config" \
            APPDATA="$default_home/AppData" \
            SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
            --game "$game_id" --verbose --boot-probe \
            --boot-probe-frames 0 2>&1) || {
            printf '%s\n' "$default_output" >&2
            exit 1
        }
        if ! printf '%s\n' "$default_output" | \
                grep -Fq "startup game=$game_id mode=direct platform=auto data=default search roots" ||
           ! printf '%s\n' "$default_output" | \
                grep -Fq "selected game=$game_id platform=FM Towns" ||
           ! printf '%s\n' "$default_output" | \
                grep -Fq "$default_data_root/$game_id/"; then
            printf '%s\n' "$default_output" >&2
            printf 'FAIL: bare --game %s did not resolve original media from the default per-game data directory\n' \
                "$game_id" >&2
            exit 1
        fi
        printf 'PASS: bare --game %s resolves original FM Towns media from ~/.firestaff/data/%s\n' \
            "$game_id" "$game_id"
    fi
done

ln -s "$mac_archive" "$combined_root/$(basename "$mac_archive")"
ln -s "$towns_archive" "$combined_root/$(basename "$towns_archive")"
combined_auto=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --game dm2 --data-dir "$combined_root" \
    --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
    printf '%s\n' "$combined_auto" >&2
    exit 1
}
case "$combined_auto" in
    *"startup game=dm2 mode=direct platform=auto"*"platform=FM Towns edition=fmtowns-ja matched source="*"platform=Macintosh edition=mac-en-retail matched source="*"selected game=dm2 platform=FM Towns edition=fmtowns-ja source="*) ;;
    *) echo "FAIL: DM2 AUTO did not prefer FM Towns when Mac retail was also present" >&2; exit 1 ;;
esac
combined_mac=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
    --game dm2 --platform mac --data-dir "$combined_root" \
    --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
    printf '%s\n' "$combined_mac" >&2
    exit 1
}
case "$combined_mac" in
    *"selected game=dm2 platform=Macintosh edition=mac-en-retail source="*) ;;
    *) echo "FAIL: explicit DM2 Macintosh choice was overridden by FM Towns default" >&2; exit 1 ;;
esac

# A bare game selection must not inherit an obsolete platform from saved
# launcher settings. Check the other two original FM Towns archives when
# they are staged, using the same direct CLI path as the DM2 check above.
if [ -f "$dm1_towns_archive" ]; then
    dm1_auto=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game dm1 --data-dir "$dm1_towns_archive" \
        --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
        printf '%s\n' "$dm1_auto" >&2
        exit 1
    }
    case "$dm1_auto" in
        *"startup game=dm1 mode=direct platform=auto"*"selected game=dm1 platform=FM Towns edition="*) ;;
        *) echo "FAIL: bare DM1 did not select original FM Towns media" >&2; exit 1 ;;
    esac
    dm1_natural_debug=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game dm1 --data-dir "$dm1_towns_archive" --debug --duration 0 2>&1) || {
        printf '%s\n' "$dm1_natural_debug" >&2
        exit 1
    }
    printf '%s\n' "$dm1_natural_debug" | python3 -c '
import re
import sys

trace = sys.stdin.read()
frames = [
    (int(step), int(elapsed), int(vblanks))
    for step, elapsed, vblanks in re.findall(
        r"startup-source-frame game=dm1 platform=fm-towns phase=title "
        r"frame=(\d+)/20 elapsed-ms=(\d+) source-wait-vblanks=(\d+)",
        trace,
    )
]
complete = re.search(
    r"startup-source-complete game=dm1 platform=fm-towns phase=title "
    r"frames=20 source-vblanks=21 elapsed-ms=(\d+)",
    trace,
)
swsh = re.search(
    r"startup-source-complete game=dm1 phase=swsh-intro "
    r"frames=(\d+) source-vblanks=(\d+) elapsed-ms=(\d+)",
    trace,
)
if ([step for step, _, _ in frames] != list(range(20)) or
        sum(vblanks for _, _, vblanks in frames) != 18 or
        complete is None or int(complete.group(1)) < 350 or
        (swsh is not None and
         (int(swsh.group(1)) != 17 or int(swsh.group(2)) != 30 or
          int(swsh.group(3)) < 3000))):
    raise SystemExit("FAIL: DM1 debug trace did not preserve FM Towns title cadence")
print("PASS: DM1 debug trace records the FM Towns title source cadence")
if swsh is not None:
    print("PASS: DM1 debug trace records the SWSH source intro cadence")
' || {
        printf '%s\n' "$dm1_natural_debug" >&2
        exit 1
    }
fi
if [ -f "$dm1_pc_archive" ]; then
    dm1_pc_debug=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game dm1 --platform pc --data-dir "$dm1_pc_archive" \
        --debug --duration 0 2>&1) || {
        printf '%s\n' "$dm1_pc_debug" >&2
        exit 1
    }
    printf '%s\n' "$dm1_pc_debug" | python3 -c '
import re
import sys

trace = sys.stdin.read()
swsh = re.search(
    r"startup-source-complete game=dm1 phase=swsh-intro "
    r"frames=(\d+) source-vblanks=(\d+) elapsed-ms=(\d+)",
    trace,
)
if swsh is None or int(swsh.group(1)) != 17 or int(swsh.group(2)) != 30 or int(swsh.group(3)) < 3000:
    raise SystemExit("FAIL: DM1 PC original-media debug trace did not preserve SWSH intro source cadence")
print("PASS: DM1 PC original-media debug trace records SWSH intro source cadence")
' || {
        printf '%s\n' "$dm1_pc_debug" >&2
        exit 1
    }
fi
if [ -f "$csb_towns_archive" ]; then
    csb_auto=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game csb --data-dir "$csb_towns_archive" \
        --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
        printf '%s\n' "$csb_auto" >&2
        exit 1
    }
    case "$csb_auto" in
        *"startup game=csb mode=direct platform=auto"*"selected game=csb platform=FM Towns edition="*) ;;
        *) echo "FAIL: bare CSB did not select original FM Towns media" >&2; exit 1 ;;
    esac
    if [ -d "$csb_towns_loose_root/CDATA" ] &&
       [ -d "$csb_towns_loose_root/CJDATA" ]; then
        csb_mixed_root=$(mktemp -d "${PWD}/.firestaff-csb-mixed-media.XXXXXX")
        mkdir "$csb_mixed_root/csb"
        ln -s "$csb_towns_archive" "$csb_mixed_root/csb/$(basename "$csb_towns_archive")"
        ln -s "$csb_towns_loose_root" "$csb_mixed_root/csb/fmtowns_iso"
        csb_mixed=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
            --game csb --data-dir "$csb_mixed_root" \
            --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
            printf '%s\n' "$csb_mixed" >&2
            exit 1
        }
        case "$csb_mixed" in
            *"selected game=csb platform=FM Towns edition=fmtowns-en source="*"$(basename "$csb_towns_archive")::CDATA/GRAPHICS.DAT"*) ;;
            *) echo "FAIL: CSB selected the loose tree over its complete original CD image" >&2; exit 1 ;;
        esac
        rm -rf "$csb_mixed_root"
    fi
fi

# When both authentic DM1 PC and FM Towns releases are installed, a bare
# --game request must retain FM Towns as the default. Keep the unsupported
# PC-98 archive in the same root when available to ensure discovery never
# promotes it into a selectable or launchable platform.
if [ -f "$dm1_towns_archive" ] && [ -f "$dm1_pc_archive" ]; then
    dm1_combined_root=$(mktemp -d "${PWD}/.firestaff-dm1-platforms.XXXXXX")
    ln -s "$dm1_towns_archive" "$dm1_combined_root/$(basename "$dm1_towns_archive")"
    ln -s "$dm1_pc_archive" "$dm1_combined_root/$(basename "$dm1_pc_archive")"
    if [ -f "$dm1_pc98_archive" ]; then
        ln -s "$dm1_pc98_archive" "$dm1_combined_root/$(basename "$dm1_pc98_archive")"
    fi
    dm1_auto_multi=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game dm1 --data-dir "$dm1_combined_root" \
        --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
        printf '%s\n' "$dm1_auto_multi" >&2
        exit 1
    }
    case "$dm1_auto_multi" in
        *"selected game=dm1 platform=FM Towns edition="*) ;;
        *) echo "FAIL: DM1 AUTO did not prefer FM Towns when PC media was also present" >&2; exit 1 ;;
    esac
    case "$dm1_auto_multi" in
        *"platform=PC-98"*|*"PC-98 edition="*)
            echo "FAIL: unsupported DM1 PC-98 media was surfaced by the launcher" >&2
            exit 1
            ;;
    esac
    dm1_pc_multi=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$firestaff_cli" \
        --game dm1 --platform pc --data-dir "$dm1_combined_root" \
        --verbose --boot-probe --boot-probe-frames 0 2>&1) || {
        printf '%s\n' "$dm1_pc_multi" >&2
        exit 1
    }
    case "$dm1_pc_multi" in
        *"selected game=dm1 platform=PC edition="*) ;;
        *) echo "FAIL: explicit DM1 PC selection was overridden by FM Towns AUTO" >&2; exit 1 ;;
    esac
    echo "PASS: DM1 AUTO prefers FM Towns over PC and ignores unsupported PC-98 media"
fi

echo "PASS: original-media startup diagnostics and FM Towns defaults"
