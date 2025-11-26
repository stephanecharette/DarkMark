#!/bin/bash
# DarkMark (C) 2019 Stephane Charette <stephanecharette@gmail.com>
# Usage: ./build_icns.sh darkmark_logo_mac.png

INPUT_FILE="$1"
ICONSET_NAME="darkmark.iconset"

mkdir -p "$ICONSET_NAME"

# sizes
sips -z 16 16     "$INPUT_FILE" --out "${ICONSET_NAME}/icon_16x16.png"
sips -z 32 32     "$INPUT_FILE" --out "${ICONSET_NAME}/icon_16x16@2x.png"
sips -z 32 32     "$INPUT_FILE" --out "${ICONSET_NAME}/icon_32x32.png"
sips -z 64 64     "$INPUT_FILE" --out "${ICONSET_NAME}/icon_32x32@2x.png"
sips -z 128 128   "$INPUT_FILE" --out "${ICONSET_NAME}/icon_128x128.png"
sips -z 256 256   "$INPUT_FILE" --out "${ICONSET_NAME}/icon_128x128@2x.png"
sips -z 256 256   "$INPUT_FILE" --out "${ICONSET_NAME}/icon_256x256.png"
sips -z 512 512   "$INPUT_FILE" --out "${ICONSET_NAME}/icon_256x256@2x.png"
sips -z 512 512   "$INPUT_FILE" --out "${ICONSET_NAME}/icon_512x512.png"
sips -z 1024 1024 "$INPUT_FILE" --out "${ICONSET_NAME}/icon_512x512@2x.png"

# to icns convert
iconutil -c icns "$ICONSET_NAME"

# cleanup
rm -rf "$ICONSET_NAME"