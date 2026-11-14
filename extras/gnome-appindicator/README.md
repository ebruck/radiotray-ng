# GNOME AppIndicator bridge for RadioTray-NG

RadioTray-NG's `DirectSni` backend deliberately exports a tiny DBusMenu object because the GNOME AppIndicator extension requires both an indicator ID and a menu path before it considers a StatusNotifierItem ready.

RadioTray-NG does **not** want GNOME Shell to render that DBusMenu. Its real menu is the native GTK menu built by `AppindicatorGui`, because that preserves the existing station artwork and GTK layout. Stock GNOME AppIndicator click handling opens the DBusMenu on primary/right click, so the shim appears as an empty grey popup before RadioTray-NG opens its own menu.

The DBusMenu bridge item is therefore exported as **hidden**. GNOME Shell still counts it for `numMenuItems`, so primary-click can open the DBusMenu root and deliver the root `opened` event, but there is no visible Shell menu row to draw behind the native GTK popup.

`DirectSni` exports `StatusNotifierItem.Activate` for the RadioTray-specific GNOME bridge. The v4 bridge invokes that D-Bus method directly for left/right release events; middle click is suppressed. The hidden DBusMenu item remains only as a readiness/fallback bridge and is not rendered.

Run:

```bash
./extras/gnome-appindicator/fix-radiotray-appindicator.sh
```

The helper modifies only the `radiotray-ng` indicator path in the user's copy of `indicatorStatusIcon.js`. Version 4 consumes **primary (left)** and **secondary (right)** presses, identifies RadioTray-NG defensively from its id/title/uniqueId, and calls the generated D-Bus `ActivateAsync(x, y)` proxy directly on the first button release. This bypasses GNOME AppIndicator's normal activation-token/double-click path entirely. **Middle click is consumed and deliberately does nothing.** Other indicators continue through the extension's original handler unchanged.

On GNOME Wayland, log out and back in once after applying the patch.

The helper never edits `/usr/share/gnome-shell/extensions` directly. If the AppIndicator extension is installed system-wide, it creates a per-user copy first and patches that copy.

The helper is idempotent: if the RadioTray marker is already present it exits without modifying the extension again. It keeps timestamped backups under the per-user extension directory before changing JavaScript.
