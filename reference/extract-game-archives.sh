#!/usr/bin/env bash
# Extraherar alla unika spelarkiv från ~/Downloads till ~/.firestaff/data/
# Skapar nya <version>-kataloger vid sidan av befintliga dm1/, csb/, etc.
# Loggar allt till ~/.firestaff/data/.extract-log.md
#
# Befintliga kataloger som INTE får röras (användaren har dessa staged):
#   dm1/            PC 3.4 English (kanonisk)
#   dm1-multilingual/ PC 3.4 Multilingual
#   csb/            PC 3.4 English
#   dm2/            PC English + data/
#   nexus/          Saturn + 138 DMDF/DGN-filer
#   theron/         PC-Engine US+JP ISO + OGG

set -euo pipefail

DLD="$HOME/Downloads"
DST="$HOME/.firestaff/data"
LOG="$DST/.extract-log.md"
TS=$(date -u +%FT%TZ)

mkdir -p "$DST"

log() {
  local msg="[$(date +%H:%M:%S)] $*"
  echo "$msg"
  echo "$msg" >> "$LOG"
}

extract_7z() {
  local archive="$1"
  local target="$2"
  mkdir -p "$target"
  log "  7z  $(basename "$archive") -> ${target#$HOME/}"
  7z x "$archive" -o"$target" -y -bb0 >/dev/null
}

extract_rar() {
  local archive="$1"
  local target="$2"
  mkdir -p "$target"
  log "  rar $(basename "$archive") -> ${target#$HOME/}"
  unrar x -y -idq "$archive" "$target/" 2>&1 | grep -v "^$" | head -20 || true
}

extract_zip() {
  local archive="$1"
  local target="$2"
  mkdir -p "$target"
  log "  zip $(basename "$archive") -> ${target#$HOME/}"
  unzip -q -o "$archive" -d "$target" 2>&1 | grep -v "mismatching.*local.*filename\|continuing with" | head -10 || true
}

extract_sit() {
  local archive="$1"
  local target="$2"
  mkdir -p "$target"
  log "  sit $(basename "$archive") -> ${target#$HOME/}"
  # -D (no directory) krävs för StuffIt-formatet, annars "Couldn't open archive"
  unar -f -D -o "$target" "$archive" >/dev/null 2>&1 || \
    log "    WARN: unar misslyckades, försöker bsdtar"
}

# Initiera logg
cat > "$LOG" <<EOF
# Game-data extraction log

**Startad:** $TS
**Källa:** $DLD
**Mål:** $DST
**Verktyg:** 7z $(7z | head -2 | tail -1), unar $(unar --version 2>&1 | head -1), unrar, unzip

EOF

log "=== Startar extraktion ==="

# === DM1 (32 arkiv) ===
DM1_DST="$DST/dm1-extras"
mkdir -p "$DM1_DST"
log ""
log "=== DM1 (32 arkiv) → $DM1_DST ==="

# Atari ST v1.1
extract_7z "$DLD/Dungeon Master for Atari ST version 1.1 English.7z" "$DM1_DST/atari-st-1.1-en"
# Atari ST v1.2
extract_7z "$DLD/Dungeon Master for Atari ST version 1.2 English.7z" "$DM1_DST/atari-st-1.2-en"
# Amiga 2.0 En/Fr/De + 2.2 En
extract_7z "$DLD/Dungeon Master for Amiga version 2.0 English.7z" "$DM1_DST/amiga-2.0-en"
extract_7z "$DLD/Dungeon Master for Amiga version 2.0 French [BarryB].7z" "$DM1_DST/amiga-2.0-fr"
extract_7z "$DLD/Dungeon Master for Amiga version 2.0 German.7z" "$DM1_DST/amiga-2.0-de"
extract_7z "$DLD/Dungeon Master for Amiga version 2.2 English.7z" "$DM1_DST/amiga-2.2-en"
# Apple IIGS 2.1
extract_7z "$DLD/Dungeon Master for Apple IIGS version 2.1 English.7z" "$DM1_DST/apple-iigs-2.1-en"
# PC 3.4 English 3.5" + 5.25"
extract_7z "$DLD/Dungeon Master for PC version 3.4 English 3.5inch.7z" "$DM1_DST/pc-3.4-en-3.5in"
extract_7z "$DLD/Dungeon Master for PC version 3.4 English 5.25inch.7z" "$DM1_DST/pc-3.4-en-5.25in"
# PC 3.4 Multilingual 3.5" (befintlig dm1-multilingual har redan detta; extraherar ändå som referens)
extract_7z "$DLD/Dungeon Master for PC version 3.4 English, French, German 3.5inch.7z" "$DM1_DST/pc-3.4-multi-3.5in"
# PC-98 2.0 JP A/B
extract_7z "$DLD/Dungeon Master for PC-9801 version 2.0 Japanese A.7z" "$DM1_DST/pc98-2.0-jp-a"
extract_7z "$DLD/Dungeon Master for PC-9801 version 2.0 Japanese B.7z" "$DM1_DST/pc98-2.0-jp-b"
# X68000 3.0 JP
extract_7z "$DLD/Dungeon Master for X68000 version 3.0 Japanese.7z" "$DM1_DST/x68000-3.0-jp"
# Japan (En,Ja)
extract_7z "$DLD/Dungeon Master (Japan) (En,Ja).7z" "$DM1_DST/japan-en-ja"
extract_7z "$DLD/Dungeon Master (Japan) (En,Ja) (Rev 1).7z" "$DM1_DST/japan-en-ja-rev1"
# FM-Towns OpTiMaL
extract_rar "$DLD/Dungeon.Master.ISO.FM-Towns-OpTiMaL.rar" "$DM1_DST/fm-towns-optimal"
# Game,* collections (små, troligen DMS-format)
extract_7z "$DLD/Game,Dungeon_Master,Amiga,Software.7z" "$DM1_DST/legacy-amiga-dms"
extract_7z "$DLD/Game,Dungeon_Master,Apple_IIGS,Software.7z" "$DM1_DST/legacy-apple-iigs"
extract_7z "$DLD/Game,Dungeon_Master,Atari_ST,Software.7z" "$DM1_DST/legacy-atari-st"
extract_7z "$DLD/Game,Dungeon_Master,DOS,Software.7z" "$DM1_DST/legacy-dos"
extract_7z "$DLD/Game,Dungeon_Master,JP,X68000,Software.7z" "$DM1_DST/legacy-jp-x68000"
extract_7z "$DLD/Game,Dungeon_Master,PC-9801,Software.7z" "$DM1_DST/legacy-jp-pc98"
extract_7z "$DLD/Game,Dungeon_Master,Super_NES,Software.7z" "$DM1_DST/legacy-snes"
# DMFiles-stil zips
extract_zip "$DLD/Dungeon-Master_Amiga_EN.zip" "$DM1_DST/dmfiles-amiga-en"
extract_zip "$DLD/Dungeon-Master_Amiga_EN_Version-20.zip" "$DM1_DST/dmfiles-amiga-en-v20"
extract_zip "$DLD/Dungeon-Master_Apple-IIgs_EN.zip" "$DM1_DST/dmfiles-apple-iigs-en"
extract_zip "$DLD/Dungeon-Master_Atari-ST_DE_Version-12-alt.zip" "$DM1_DST/dmfiles-atari-st-de-v12-alt"
extract_zip "$DLD/Dungeon-Master_Atari-ST_DE_Version-12.zip" "$DM1_DST/dmfiles-atari-st-de-v12"
extract_zip "$DLD/Dungeon-Master_Atari-ST_EN_Version-12.zip" "$DM1_DST/dmfiles-atari-st-en-v12"
extract_zip "$DLD/Dungeon-Master_Atari-ST_FR_Version-13.zip" "$DM1_DST/dmfiles-atari-st-fr-v13"
extract_zip "$DLD/Dungeon-Master_DOS_EN.zip" "$DM1_DST/dmfiles-dos-en"
extract_zip "$DLD/Dungeon-Master_DOS_EN_Version-34.zip" "$DM1_DST/dmfiles-dos-en-v34"
extract_zip "$DLD/Dungeon-Master_DOS_FR.zip" "$DM1_DST/dmfiles-dos-fr"
extract_zip "$DLD/Dungeon-Master_FM-Towns_JA-EN.zip" "$DM1_DST/dmfiles-fm-towns-ja-en"
extract_zip "$DLD/Dungeon-Master_PC-98_EN.zip" "$DM1_DST/dmfiles-pc98-en"
extract_zip "$DLD/Dungeon-Master_Sharp-X68000_EN.zip" "$DM1_DST/dmfiles-x68000-en"
# CSB Expansion Set 1 (DM1-filer ingår ibland)
extract_zip "$DLD/Dungeon-Master-Chaos-Strikes-Back---Expansion-Set-1_Amiga_EN.zip" "$DM1_DST/csxb-expansion1-amiga-en"
extract_zip "$DLD/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_FM-Towns_JA-EN.zip" "$DM1_DST/csxb-expansion1-fm-towns-ja-en"
extract_zip "$DLD/Dungeon-Master-Chaos-Strikes-Back-Expansion-Set-1_Sharp-X68000_EN.zip" "$DM1_DST/csxb-expansion1-x68000-en"

# === CSB (16 arkiv) ===
CSB_DST="$DST/csb-extras"
mkdir -p "$CSB_DST"
log ""
log "=== CSB (16 arkiv) → $CSB_DST ==="

extract_7z "$DLD/Chaos Strikes Back for Amiga 3.5 CTRaw.7z" "$CSB_DST/amiga-3.5-ctraw-en"
extract_7z "$DLD/Chaos Strikes Back for Amiga version 3.1 English, French, German.7z" "$CSB_DST/amiga-3.1-multi"
extract_7z "$DLD/Chaos Strikes Back for Amiga version 3.3 English, French, German.7z" "$CSB_DST/amiga-3.3-multi"
extract_7z "$DLD/Chaos Strikes Back for Amiga Utility Disk 2 English.7z" "$CSB_DST/amiga-util-disk2-en"
extract_7z "$DLD/Chaos Strikes Back for Amiga Utility Disk 2 French.7z" "$CSB_DST/amiga-util-disk2-fr"
extract_7z "$DLD/Chaos Strikes Back for Amiga Utility Disk 2 German.7z" "$CSB_DST/amiga-util-disk2-de"
extract_7z "$DLD/Chaos Strikes Back for Amiga Utility Disk 3 English.7z" "$CSB_DST/amiga-util-disk3-en"
extract_7z "$DLD/Chaos Strikes Back for Amiga Utility Disk 3 German.7z" "$CSB_DST/amiga-util-disk3-de"
extract_7z "$DLD/Chaos Strikes Back for PC-9801 version 3.1 Japanese.7z" "$CSB_DST/pc98-3.1-jp"
extract_7z "$DLD/Dungeon Master - Chaos Strikes Back (Japan) (En,Ja).7z" "$CSB_DST/japan-en-ja"
extract_rar "$DLD/Chaos Strikes Back for FM-Towns.rar" "$CSB_DST/fm-towns"
extract_7z "$DLD/Game,Chaos_Strikes_Back,Amiga,Software.7z" "$CSB_DST/legacy-amiga-dms"
extract_7z "$DLD/Game,Chaos_Strikes_Back,Atari_ST,Software.7z" "$CSB_DST/legacy-atari-st"
extract_7z "$DLD/Game,Chaos_Strikes_Back,Atari_ST,Version_2-1_Source,Disassembly,Software.7z" "$CSB_DST/legacy-atari-st-v21-source"
extract_7z "$DLD/Game,Chaos_Strikes_Back,JP,PC-9801,Software.7z" "$CSB_DST/legacy-jp-pc98"
extract_7z "$DLD/Game,Chaos_Strikes_Back,JP,X68000,Software.7z" "$CSB_DST/legacy-jp-x68000"

# === DM2 (12 arkiv) ===
DM2_DST="$DST/dm2-extras"
mkdir -p "$DM2_DST"
log ""
log "=== DM2 (12 arkiv) → $DM2_DST ==="

extract_sit "$DLD/Dungeon Master II for Macintosh English.sit" "$DM2_DST/mac-en-v1"
extract_rar "$DLD/Dungeon Master II for Mega CD Japanese.rar" "$DM2_DST/mega-cd-jp"
extract_rar "$DLD/Dungeon Master II for PC French.rar" "$DM2_DST/pc-fr"
extract_rar "$DLD/Dungeon Master II for PC German.rar" "$DM2_DST/pc-de"
extract_rar "$DLD/Dungeon Master II for PC-9821.rar" "$DM2_DST/pc9821-jp"
extract_zip "$DLD/Dungeon-Master-II-Skullkeep_Amiga_EN.zip" "$DM2_DST/amiga-en"
extract_zip "$DLD/Dungeon-Master-II-Skullkeep_DOS_EN.zip" "$DM2_DST/dos-en"
extract_zip "$DLD/Dungeon-Master-II-Skullkeep_DOS_FR.zip" "$DM2_DST/dos-fr"
extract_zip "$DLD/Dungeon-Master-II-Skullkeep_FM-Towns_JA.zip" "$DM2_DST/fm-towns-ja"
extract_zip "$DLD/Dungeon-Master-II-Skullkeep_Mac_EN.zip" "$DM2_DST/mac-en-zip"
extract_sit "$DLD/Dungeon-Master-II-Skullkeep_Mac_FR.sit" "$DM2_DST/mac-fr"
extract_zip "$DLD/Dungeon-Master-II-Skullkeep_Mac_JA.zip" "$DM2_DST/mac-ja"

# === Nexus (1 arkiv) ===
NEXUS_DST="$DST/nexus-extras"
mkdir -p "$NEXUS_DST"
log ""
log "=== Nexus (1 arkiv) → $NEXUS_DST ==="
extract_zip "$DLD/Dungeon-Master-Nexus_SEGA-Saturn_JA.zip" "$NEXUS_DST/saturn-ja"

# === Theron's Quest (3 arkiv) ===
THERON_DST="$DST/theron-extras"
mkdir -p "$THERON_DST"
log ""
log "=== Theron's Quest (3 arkiv) → $THERON_DST ==="
extract_7z "$DLD/Dungeon Master - Theron's Quest (Japan).7z" "$THERON_DST/japan"
extract_7z "$DLD/Dungeon Master - Theron's Quest (USA).7z" "$THERON_DST/usa"
extract_rar "$DLD/Theron's Quest for PC-Engine (US and Japanese versions).rar" "$THERON_DST/pc-engine"

log ""
log "=== Extraktion klar ==="
echo ""
echo "=== Slutstatus ==="
echo "Totalt antal extraherade underkataloger:"
find "$DST" -mindepth 2 -maxdepth 2 -type d | wc -l
echo ""
echo "Total storlek på ~/.firestaff/data/:"
du -sh "$DST"
echo ""
echo "Logg: $LOG"
