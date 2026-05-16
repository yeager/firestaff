#!/bin/bash
# Run on macOS to create .icns from PNGs
ICONSET=firestaff.iconset
mkdir -p "$ICONSET"
cp icons/firestaff_32.png "$ICONSET/icon_32x32.png"
cp icons/firestaff_128.png "$ICONSET/icon_128x128.png"
cp icons/firestaff_256.png "$ICONSET/icon_256x256.png"
cp icons/firestaff_512.png "$ICONSET/icon_512x512.png"
# Also need @2x variants
cp icons/firestaff_256.png "$ICONSET/icon_128x128@2x.png"
cp icons/firestaff_512.png "$ICONSET/icon_256x256@2x.png"
iconutil -c icns "$ICONSET" -o firestaff.icns
rm -rf "$ICONSET"
echo "Created firestaff.icns"
