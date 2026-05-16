#!/bin/bash
# Create Firestaff data directory structure
DIR="${1:-$HOME/.firestaff/data}"
echo "Creating data directories in: $DIR"

mkdir -p "$DIR/dm1"
mkdir -p "$DIR/csb"
mkdir -p "$DIR/dm2"
mkdir -p "$DIR/dm1-multilingual"
mkdir -p "$DIR/nexus"

cat > "$DIR/dm1/README.txt" << 'EOF'
Place DM1 PC DOS (PC-34) data files here:
  GRAPHICS.DAT  (~355 KB)
  DUNGEON.DAT   (~33 KB)
EOF

cat > "$DIR/csb/README.txt" << 'EOF'
Place Chaos Strikes Back data files here:
  GRAPHICS.DAT
  DUNGEON.DAT
EOF

cat > "$DIR/dm2/README.txt" << 'EOF'
Place Dungeon Master II: Skullkeep data files here:
  GRAPHICS.DAT
  DUNGEON.DAT
EOF

cat > "$DIR/dm1-multilingual/README.txt" << 'EOF'
Place PC-34 Multilingual data files here for French/German text:
  GRAPHICS.DAT   (shared)
  DUNGEON.DAT    (English)
  DUNGEONF.DAT   (French)
  DUNGEONG.DAT   (German)
EOF

cat > "$DIR/nexus/README.txt" << 'EOF'
Place extracted DM Nexus Saturn files here.
Use: python3 tools/extract_nexus_iso.py
EOF

echo "Done. Place your game data files in the directories above."
echo "Run: firestaff --validate to check."
