# NetHack 5.0 for Powkiddy X55 (JELOS)

A port of NetHack 5.0 for the Powkiddy X55 handheld running JELOS.

## Tested Configuration

- **Device:** Powkiddy X55 (Rockchip RK3566, 1280x720 screen)
- **OS:** JELOS 20231029
- **NetHack:** 5.0.0

This port uses a custom SDL2 window system designed for handheld play with
gamepad controls. No keyboard required.

## Features

- Full NetHack 5.0 gameplay
- Tile graphics with zoom support (L2/R2 to zoom in/out)
- Gamepad-only controls with command menu
- Two tileset options: Official 16x16 or Nevanda 32x32

## Controls

| Button | Action |
|--------|--------|
| D-pad / Left Stick | Move (8 directions) |
| A | Confirm / Rest (`.`) |
| B | Cancel (ESC) |
| X | Pick up (`,`) |
| Y | Search (`s`) |
| L1 / R1 | Page up/down in menus and message history |
| L2 / R2 | Zoom out/in (tile mode) |
| Start | Command menu (access all commands) |
| Select | Inventory (`i`) |

The **Command Menu** (Start button) provides access to all NetHack commands
through a searchable list, so you never need a physical keyboard.

## Installation

1. Enable SSH in JELOS network settings
2. Create the game directory:
   ```bash
   ssh root@<device-ip> "mkdir -p /storage/roms/ports/nethack"
   ```
3. Copy the game files:
   ```bash
   scp nethack root@<device-ip>:/storage/roms/ports/nethack/
   scp nevanda_5.0_v2.png root@<device-ip>:/storage/roms/ports/nethack/
   scp dat/* root@<device-ip>:/storage/roms/ports/nethack/
   ```
4. Create the launcher script `/storage/roms/ports/NetHack.sh`:
   ```bash
   #!/bin/bash
   cd /storage/roms/ports/nethack
   ./nethack
   ```
5. Make it executable:
   ```bash
   ssh root@<device-ip> "chmod +x /storage/roms/ports/NetHack.sh"
   ```
6. The game will appear in the Ports section of JELOS

## Building from Source

Requires an aarch64 cross-compiler (tested with `aarch64-linux-gnu-gcc` 14.2.0).

1. Set up SDL2 headers and libraries for cross-compilation in `targets/aarch64/`
2. From `nethack-src/src/`, run:
   ```bash
   make CROSS_TO_AARCH64=1 WANT_WIN_SDL2=1 all
   ```
3. The binary will be at `targets/aarch64/nethack`

See `nethack-src/sys/unix/hints/include/cross-pre2.500` for build configuration
details.

## Tilesets

Two tilesets are included:

| Tileset | File | Tile Size | Description |
|---------|------|-----------|-------------|
| Nevanda | `nevanda_5.0_v2.png` | 32x32 | Default, detailed graphics |
| Official | `Nhtiles-500-16x16.png` | 16x16 | Classic NetHack tiles |

To switch tilesets, modify `SDL2_TILE_WIDTH`/`SDL2_TILE_HEIGHT` in
`win/sdl2/winsdl2.h` and the tileset path in `win/sdl2/winsdl2.c`, then rebuild.

## Known Limitations

- On-screen keyboard not yet implemented (text input like character names
  requires SSH or pre-configured options file)
- Designed specifically for 1280x720 screens

## License

This port is released under the **NetHack General Public License** (NGPL).
See `dat/license` for the full license text.

NetHack is Copyright (c) Stichting Mathematisch Centrum and others.
This port's SDL2 window code is new work released under the same license.

You are free to copy, distribute, and modify this software under the terms
of the NGPL. If you distribute modified versions, you must:
- Include prominent notices stating what you changed and when
- Make source code available
- Keep the same license

## Credits

- **NetHack DevTeam** - NetHack 5.0
- **Nevanda tileset** - Community contribution
- **SDL2 port** - Created for Powkiddy X55/JELOS
