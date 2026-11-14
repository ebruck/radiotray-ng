#!/usr/bin/env bash
set -euo pipefail

UUID="appindicatorsupport@rgcjonas.gmail.com"
USER_EXT="${HOME}/.local/share/gnome-shell/extensions/${UUID}"
SYSTEM_EXT="/usr/share/gnome-shell/extensions/${UUID}"
JS_REL="indicatorStatusIcon.js"
MARKER="RadioTray-NG native-menu click bridge v6"
STAMP="$(date +%Y%m%d-%H%M%S)"

say() { printf '%s\n' "$*"; }
die() { say "ERROR: $*" >&2; exit 1; }

if [[ ${EUID:-$(id -u)} -eq 0 ]]; then
    die "Run this as your normal desktop user, not with sudo."
fi

say "== RadioTray-NG GNOME AppIndicator click fix v6 =="
say

if [[ -f "${USER_EXT}/${JS_REL}" ]]; then
    EXT_DIR="$USER_EXT"
elif [[ -f "${SYSTEM_EXT}/${JS_REL}" ]]; then
    say "Creating a per-user copy of the installed AppIndicator extension..."
    mkdir -p "$(dirname "$USER_EXT")"
    rm -rf "${USER_EXT}.new"
    cp -a "$SYSTEM_EXT" "${USER_EXT}.new"
    rm -rf "$USER_EXT"
    mv "${USER_EXT}.new" "$USER_EXT"
    EXT_DIR="$USER_EXT"
else
    die "GNOME AppIndicator extension not found."
fi

JS="${EXT_DIR}/${JS_REL}"
BACKUP_DIR="${EXT_DIR}/.radiotray-ng-backups"
BACKUP="${BACKUP_DIR}/indicatorStatusIcon.js.${STAMP}"

mkdir -p "$BACKUP_DIR"
cp -a "$JS" "$BACKUP"
say "Backup of current file: $BACKUP"

# V6 no longer requires a pristine system copy.  It repairs the installed file
# in-place by replacing the complete button-handler region between the stock
# press and scroll methods.  That removes every broken v1-v5 fragment at once.
python3 - "$JS" <<'PY'
from pathlib import Path
import sys

path = Path(sys.argv[1])
s = path.read_text()

press_sig = "    vfunc_button_press_event(event) {"
scroll_sig = "    vfunc_scroll_event(event) {"

press_start = s.find(press_sig)
scroll_start = s.find(scroll_sig)

if press_start < 0:
    raise SystemExit("ERROR: Could not find vfunc_button_press_event(event)")
if scroll_start < 0 or scroll_start <= press_start:
    raise SystemExit("ERROR: Could not find the expected vfunc_scroll_event(event) after the press handler")

replacement = r'''    vfunc_button_press_event(event) {
        // RadioTray-NG native-menu click bridge v6
        const rtId = String(this._indicator?.id ?? '').toLowerCase();
        const rtTitle = String(this._indicator?.title ?? '').toLowerCase();
        const rtUniqueId = String(this._indicator?.uniqueId ?? '').toLowerCase();
        const isRadioTray =
            rtId === 'radiotray-ng' ||
            rtTitle === 'radiotray-ng' ||
            rtUniqueId.includes('radiotray-ng') ||
            rtUniqueId.includes('radiotray');

        if (isRadioTray) {
            if (this._waitDoubleClickPromise)
                this._waitDoubleClickPromise.cancel();

            const button = event.get_button();

            // Stop all stock click handling for RadioTray-NG. LEFT and RIGHT
            // are completed on button release; MIDDLE is deliberately ignored.
            if (button === Clutter.BUTTON_PRIMARY ||
                button === Clutter.BUTTON_SECONDARY ||
                button === Clutter.BUTTON_MIDDLE)
                return Clutter.EVENT_STOP;
        }

        // Original AppIndicator behaviour for every other indicator.
        if (this._waitDoubleClickPromise)
            this._waitDoubleClickPromise.cancel();

        if (event.get_button() === Clutter.BUTTON_MIDDLE) {
            if (Main.panel.menuManager.activeMenu)
                Main.panel.menuManager._closeMenu(true, Main.panel.menuManager.activeMenu);
            this._indicator.secondaryActivate(event.get_time(), ...event.get_coords());
            return Clutter.EVENT_STOP;
        }

        if (event.get_button() === Clutter.BUTTON_SECONDARY) {
            this.menu.toggle();
            return Clutter.EVENT_PROPAGATE;
        }

        const doubleClickHandled = this._maybeHandleDoubleClick(event);
        if (doubleClickHandled === Clutter.EVENT_PROPAGATE &&
            event.get_button() === Clutter.BUTTON_PRIMARY &&
            this.menu.numMenuItems) {
            if (this._indicator.supportsActivation !== false)
                this._waitForDoubleClick().catch(logError);
            else
                this.menu.toggle();
        }

        return Clutter.EVENT_PROPAGATE;
    }

    vfunc_button_release_event(event) {
        const rtId = String(this._indicator?.id ?? '').toLowerCase();
        const rtTitle = String(this._indicator?.title ?? '').toLowerCase();
        const rtUniqueId = String(this._indicator?.uniqueId ?? '').toLowerCase();
        const isRadioTray =
            rtId === 'radiotray-ng' ||
            rtTitle === 'radiotray-ng' ||
            rtUniqueId.includes('radiotray-ng') ||
            rtUniqueId.includes('radiotray');

        if (isRadioTray) {
            const button = event.get_button();

            if (button === Clutter.BUTTON_MIDDLE)
                return Clutter.EVENT_STOP;

            if (button === Clutter.BUTTON_PRIMARY ||
                button === Clutter.BUTTON_SECONDARY) {
                if (this._waitDoubleClickPromise)
                    this._waitDoubleClickPromise.cancel();

                if (Main.panel.menuManager.activeMenu)
                    Main.panel.menuManager._closeMenu(
                        true, Main.panel.menuManager.activeMenu);

                const [x, y] = event.get_coords();

                // Direct D-Bus call: exactly one LEFT/RIGHT release maps to
                // exactly one RadioTray-NG Activate(x,y).
                this._indicator._proxy.ActivateAsync(
                    x, y, this._indicator.cancellable).catch(logError);

                return Clutter.EVENT_STOP;
            }
        }

        return Clutter.EVENT_PROPAGATE;
    }

'''

s = s[:press_start] + replacement + s[scroll_start:]

# Remove any old RadioTray bridge marker that may have survived outside the
# replaced handler region.  The v6 marker itself is preserved.
for marker in (
    "RadioTray-NG native-menu click bridge v1",
    "RadioTray-NG native-menu click bridge v2",
    "RadioTray-NG native-menu click bridge v3",
    "RadioTray-NG native-menu click bridge v4",
    "RadioTray-NG native-menu click bridge v5",
):
    s = s.replace(marker, "obsolete RadioTray-NG bridge marker removed")

path.write_text(s.rstrip("\n") + "\n")
PY

grep -qF "$MARKER" "$JS" || die "V6 marker missing after repair."
grep -q "vfunc_button_release_event(event)" "$JS" || die "V6 release handler missing."
grep -q "_proxy.ActivateAsync" "$JS" || die "V6 direct Activate call missing."

# The broken migration left duplicated handler fragments.  These checks make
# sure only the expected handler definitions remain.
PRESS_COUNT="$(grep -c '^[[:space:]]*vfunc_button_press_event(event)' "$JS" || true)"
RELEASE_COUNT="$(grep -c '^[[:space:]]*vfunc_button_release_event(event)' "$JS" || true)"
SCROLL_COUNT="$(grep -c '^[[:space:]]*vfunc_scroll_event(event)' "$JS" || true)"

[[ "$PRESS_COUNT" == "1" ]] || die "Expected exactly one button-press handler, found $PRESS_COUNT."
[[ "$RELEASE_COUNT" == "1" ]] || die "Expected exactly one button-release handler, found $RELEASE_COUNT."
[[ "$SCROLL_COUNT" == "1" ]] || die "Expected exactly one scroll handler, found $SCROLL_COUNT."

say
say "RadioTray-NG click policy v6 repaired successfully:"
say "  LEFT   -> direct Activate on first release"
say "  RIGHT  -> direct Activate on first release"
say "  MIDDLE -> ignored"
say "  SCROLL -> original AppIndicator scroll handler"
say "Modified: $JS"

ACTIVE_PATH=""
if command -v gnome-extensions >/dev/null 2>&1; then
    ACTIVE_PATH="$(gnome-extensions info "$UUID" 2>/dev/null | sed -n 's/^[[:space:]]*Path:[[:space:]]*//p' | head -n1 || true)"
fi
[[ -n "$ACTIVE_PATH" ]] && say "GNOME extension path: $ACTIVE_PATH"

say
say "Requesting extension reload..."
gnome-extensions disable "$UUID" 2>/dev/null || true
sleep 1
gnome-extensions enable "$UUID" 2>/dev/null || true

say
say "On GNOME Wayland, log out and back in once before testing."
say "Expected: LEFT/RIGHT single click -> native menu; MIDDLE -> no action; scroll -> volume."
