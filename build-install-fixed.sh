#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${ROOT}/build-fixed"

say() { printf '%s\n' "$*"; }
die() { say "ERROR: $*" >&2; exit 1; }

if [[ ${EUID:-$(id -u)} -eq 0 ]]; then
    die "Run this script as your normal desktop user, not with sudo."
fi

cd "$ROOT"

for cmd in cmake dpkg-shlibdeps dpkg-deb dpkg-query apt-get; do
    command -v "$cmd" >/dev/null 2>&1 || die "Required command not found: $cmd"
done

JOBS="${RTNG_JOBS:-$(nproc 2>/dev/null || printf '2')}"

say "[1/5] Configuring a clean Release build..."
rm -rf "$BUILD_DIR"
cmake -S "$ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release

say
say "[2/5] Building RadioTray-NG..."
cmake --build "$BUILD_DIR" --parallel "$JOBS"

# Fail early if the linker left anything unresolved.  This is more useful than
# discovering it later while CPack is generating package metadata.
for exe in "$BUILD_DIR/radiotray-ng" "$BUILD_DIR/rtng-bookmark-editor"; do
    [[ -x "$exe" ]] || die "Expected executable was not built: $exe"
    if command -v ldd >/dev/null 2>&1 && ldd "$exe" | grep -q 'not found'; then
        ldd "$exe" >&2 || true
        die "Unresolved shared-library dependency in $exe"
    fi
done

say
say "[3/5] Building Debian package with dpkg-shlibdeps-derived dependencies..."
cmake --build "$BUILD_DIR" --target package --parallel "$JOBS"

mapfile -t DEBS < <(find "$BUILD_DIR" -type f -name 'radiotray-ng*.deb' -printf '%T@ %p\n' | sort -nr | cut -d' ' -f2-)
(( ${#DEBS[@]} > 0 )) || die "Build completed but no radiotray-ng .deb was found under $BUILD_DIR"
DEB="${DEBS[0]}"

say
say "[4/5] Verifying and installing package: $DEB"
say "Package: $(dpkg-deb -f "$DEB" Package)"
say "Version: $(dpkg-deb -f "$DEB" Version)"
say "Depends: $(dpkg-deb -f "$DEB" Depends 2>/dev/null || true)"

dpkg-deb --info "$DEB" >/dev/null
dpkg-deb --contents "$DEB" >/dev/null

say
say "Simulating installation with APT..."
apt-get --simulate install "$DEB" || die "APT cannot resolve the generated package on this system; nothing was installed."

say
say "Installing/reinstalling with APT..."
# Development builds can legitimately keep the same package version while the
# source changes. Without --reinstall, APT reports "already the newest version"
# and leaves the old /usr/bin/radiotray-ng in place.
sudo apt-get install -y --reinstall "$DEB"

dpkg-query -W -f='${Status}\n' radiotray-ng 2>/dev/null | grep -qx 'install ok installed' \
    || die "radiotray-ng is not in the 'install ok installed' state after APT returned."

command -v radiotray-ng >/dev/null 2>&1 || die "Installed radiotray-ng executable is not on PATH."
command -v rtng-bookmark-editor >/dev/null 2>&1 || die "Installed bookmark editor is not on PATH."

# Confirm the installed executable is byte-for-byte the one in the package.
VERIFY_DIR="$(mktemp -d)"
trap 'rm -rf "$VERIFY_DIR"' EXIT
dpkg-deb -x "$DEB" "$VERIFY_DIR"
[[ -x "$VERIFY_DIR/usr/bin/radiotray-ng" ]] || die "Package does not contain usr/bin/radiotray-ng."
cmp -s "$VERIFY_DIR/usr/bin/radiotray-ng" "$(command -v radiotray-ng)" \
    || die "Installed radiotray-ng does not match the newly built package."
rm -rf "$VERIFY_DIR"
trap - EXIT

say
say "[5/5] Applying the RadioTray-only GNOME AppIndicator click bridge..."
"${ROOT}/extras/gnome-appindicator/fix-radiotray-appindicator.sh"

say
say "Build, package verification and installation completed successfully."
say "On GNOME Wayland, log out and back in once before testing the tray icon."
