#!/usr/bin/env bash
# =============================================================================
#  Resonance - collects the GTK runtime for the Windows installer
#
#  Run from the repository root, in the MSYS2 MINGW64 shell:
#      bash installer/collect-runtime.sh
#
#  Result (ignored by Git), read by installer/resonance.iss:
#      build/windows/bin/     resonance.exe + the DLLs it needs
#      build/windows/lib/     gdk-pixbuf image loaders (PNG, ICO...)
#      build/windows/share/   GLib schemas and the icon theme index
#
#  Only DLLs that come from MSYS2 (/mingw64) are copied. System DLLs such as
#  KERNEL32.dll or USER32.dll are part of Windows: they must never be shipped.
# =============================================================================
set -euo pipefail

MSYS_PREFIX=/mingw64
OUT=build/windows
PIXBUF_DIR=lib/gdk-pixbuf-2.0/2.10.0

if [[ "${MSYSTEM:-}" != "MINGW64" ]]; then
    echo "error: run this script in the MSYS2 MINGW64 shell" >&2
    exit 1
fi
if [[ ! -f Makefile || ! -d src ]]; then
    echo "error: run this script from the repository root" >&2
    exit 1
fi

echo "==> Building"
mingw32-make

rm -rf "$OUT"
mkdir -p "$OUT/bin" "$OUT/$PIXBUF_DIR/loaders" "$OUT/share/glib-2.0/schemas" "$OUT/share/icons"
cp bin/resonance.exe "$OUT/bin/"

# Copies every MSYS2 DLL that the given files depend on (recursively,
# since ldd already lists indirect dependencies).
copy_msys_dlls() {
    ldd "$@" | awk '{print $3}' | { grep -i "^$MSYS_PREFIX/" || true; } | sort -u | while read -r dll; do
        cp -u "$dll" "$OUT/bin/"
    done
}

echo "==> Copying DLLs of resonance.exe"
copy_msys_dlls "$OUT/bin/resonance.exe"

echo "==> Copying image loaders (without SVG: the interface uses only PNG and ICO)"
shopt -s nullglob
loaders=()
for loader in "$MSYS_PREFIX/$PIXBUF_DIR/loaders/"*.dll; do
    case "$(basename "$loader")" in
        *svg*) continue ;;
    esac
    cp "$loader" "$OUT/$PIXBUF_DIR/loaders/"
    loaders+=("$loader")
done
if (( ${#loaders[@]} > 0 )); then
    copy_msys_dlls "${loaders[@]}"
fi

# loaders.cache lists one block per loader, separated by blank lines.
# Blocks of loaders that were not copied (SVG) are dropped.
awk -v RS= -v ORS='\n\n' '!/svg/' "$MSYS_PREFIX/$PIXBUF_DIR/loaders.cache" > "$OUT/$PIXBUF_DIR/loaders.cache"

echo "==> Copying GLib schemas and icon theme index"
cp "$MSYS_PREFIX/share/glib-2.0/schemas/gschemas.compiled" "$OUT/share/glib-2.0/schemas/"
mkdir -p "$OUT/share/icons/hicolor"
cp "$MSYS_PREFIX/share/icons/hicolor/index.theme" "$OUT/share/icons/hicolor/"

echo "==> Checking for Windows system DLLs (there must be none)"
if ls "$OUT/bin" | grep -Eiq '^(kernel32|kernelbase|ntdll|user32|gdi32|advapi32|shell32|ole32|combase|msvcrt|ucrtbase|ws2_32)\.dll$'; then
    echo "error: a Windows system DLL was copied" >&2
    exit 1
fi

echo
echo "Done: $(ls "$OUT/bin" | wc -l) files in $OUT/bin, $(du -sh "$OUT" | cut -f1) in total."
echo "Next step: open installer/resonance.iss in Inno Setup and compile it (Ctrl+F9)."
