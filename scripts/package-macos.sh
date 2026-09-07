#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
make viewer
bundle="build/Tinikami.app"
mkdir -p "$bundle/Contents/MacOS" "$bundle/Contents/Frameworks" "$bundle/Contents/Resources/tinikami/animations" "$bundle/Contents/Resources/models"
cp build/creature_lab "$bundle/Contents/MacOS/creature_lab"
cp assets/tinikami/*.rgba "$bundle/Contents/Resources/tinikami/"
cp assets/tinikami/animations/*.rgba "$bundle/Contents/Resources/tinikami/animations/"
cp models/*.tbrain "$bundle/Contents/Resources/models/"
cp assets/tinikami/Tinikami.icns "$bundle/Contents/Resources/"
cp LICENSE "$bundle/Contents/Resources/LICENSE.txt"
sdl_library="$(otool -L build/creature_lab | awk '/libSDL2/{print $1; exit}')"
if [ ! -f "$sdl_library" ]; then printf '%s\n' 'Could not locate linked SDL2 library'; exit 2; fi
cp -f -L "$sdl_library" "$bundle/Contents/Frameworks/libSDL2.dylib"
chmod u+w "$bundle/Contents/Frameworks/libSDL2.dylib"
install_name_tool -id '@rpath/libSDL2.dylib' "$bundle/Contents/Frameworks/libSDL2.dylib"
install_name_tool -change "$sdl_library" '@executable_path/../Frameworks/libSDL2.dylib' "$bundle/Contents/MacOS/creature_lab"
sdl_prefix="$(sdl2-config --prefix)"
cp "$sdl_prefix/LICENSE.txt" "$bundle/Contents/Resources/SDL2-LICENSE.txt"
cat > "$bundle/Contents/Info.plist" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><dict>
<key>CFBundleExecutable</key><string>creature_lab</string>
<key>CFBundleIdentifier</key><string>org.creaturelab.workbench</string>
<key>CFBundleName</key><string>Tinikami</string>
<key>CFBundleIconFile</key><string>Tinikami.icns</string>
<key>CFBundlePackageType</key><string>APPL</string>
<key>CFBundleVersion</key><string>10</string>
<key>CFBundleShortVersionString</key><string>0.10.0</string>
<key>LSMinimumSystemVersion</key><string>14.0</string>
<key>NSHighResolutionCapable</key><true/>
</dict></plist>
PLIST
codesign --force --sign - "$bundle/Contents/Frameworks/libSDL2.dylib"
codesign --force --sign - "$bundle"
codesign --verify --deep --strict "$bundle"
archive="build/Tinikami-macOS-$(uname -m).zip"
ditto -c -k --sequesterRsrc --keepParent "$bundle" "$archive"
printf '%s\n' "Created $bundle and $archive (SDL2 and native models included; ad hoc signed)."
