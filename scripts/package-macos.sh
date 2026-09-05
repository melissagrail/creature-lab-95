#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
make viewer
bundle="build/Creature Lab.app"
mkdir -p "$bundle/Contents/MacOS"
cp build/creature_lab "$bundle/Contents/MacOS/creature_lab"
cat > "$bundle/Contents/Info.plist" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><dict>
<key>CFBundleExecutable</key><string>creature_lab</string>
<key>CFBundleIdentifier</key><string>org.creaturelab.workbench</string>
<key>CFBundleName</key><string>Creature Lab</string>
<key>CFBundlePackageType</key><string>APPL</string>
<key>CFBundleVersion</key><string>5</string>
<key>CFBundleShortVersionString</key><string>0.5.0</string>
<key>NSHighResolutionCapable</key><true/>
</dict></plist>
PLIST
printf '%s\n' "Created $bundle (uses local SDL2 installation)."
