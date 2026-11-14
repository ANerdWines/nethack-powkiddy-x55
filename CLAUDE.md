# NetHack 5.0 port for the Powkiddy X55 (JELOS)

Personal, non-commercial project. Andrew wants to play NetHack 5.0 on his
Powkiddy X55 handheld. Work in this folder.

## Target device

- Powkiddy X55: Rockchip RK3566, quad-core Cortex-A55 (aarch64), Mali-G52
- Screen: 5.5" IPS, 1280x720, landscape
- Controls: D-pad, two analog sticks, ABXY, shoulder buttons, Start/Select
- OS: **JELOS 20231029**. Stay on JELOS: do not suggest switching to ROCKNIX.
- JELOS is discontinued, so build against what this release ships and don't
  assume any future updates.

## Device details (queried via SSH)

- **IP address:** 192.168.1.73
- **SSH:** user `root`, password `qwertyuiop`
- **Kernel:** Linux RK3566-X55 4.19.219+ aarch64
- **glibc:** 2.38
- **SDL2:** 2.28.4 (`libSDL2-2.0.so.0.2800.4`)
- **SDL2_ttf:** 2.20.2 (`libSDL2_ttf-2.0.so.0.2000.2`)
- **SDL2_image:** 2.6.3
- **SDL2_mixer:** 2.6.3
- **Ports path:** `/storage/roms/ports/`
- **Game path:** `/storage/roms/ports/nethack/`
- **Launcher:** `/storage/roms/ports/NetHack.sh`

## Build environment (WSL)

- **Cross-compiler:** `aarch64-linux-gnu-gcc` 14.2.0
- **Host glibc:** 2.41 (fine for cross-compile)
- **NetHack source:** `nethack-src/` (tag NetHack-5.0.0_Released)
- **Lua:** vendored 5.4.8 in `lib/lua-5.4.8/`
- **ncurses:** cross-compiled in `targets/aarch64/ncurses/` (for tty build)

## Decisions already made

- **No PortMaster or gptokeyb dependency.** PortMaster no longer supports
  JELOS. The game reads the gamepad itself through SDL2's GameController API.
- **Custom SDL2 window port.** There is no X11 or desktop, and the game draws
  fullscreen through SDL2 (KMSDRM/whatever JELOS's SDL2 uses).
- **Text (ASCII) display first.** It must be readable at 1280x720 with the
  80x24 map plus message line and status lines. Tiles are a later, optional
  step.
- **Self-contained port folder.** Bundle any library that JELOS 20231029 does
  not provide. Check glibc and SDL2 versions on the device before choosing
  the build environment, and make sure the binary does not need a newer glibc
  than the device has.
- NetHack 5.0 uses Lua for level files. Lua is vendored in the NetHack source
  tree, so build it from there.
- **Licence:** NetHack General Public License. Free release with full source
  only. Mark modified files with prominent change notices.

## Control scheme (starting point, refine after testing)

- D-pad / left stick: move (8 directions; stick allows diagonals)
- A: confirm / `.` rest   B: cancel (ESC)
- X: pick up `,`          Y: search `s`
- L1/R1: scroll message history / menus
- Start: command menu (searchable list of all extended commands and common
  keys, so nothing needs a keyboard)
- Select: inventory `i`
- An on-screen keyboard for text prompts (name, wishes, engraving, #annotate)

## Deployment

- JELOS has built-in SSH. Enable it in JELOS network settings. User is
  `root`, and the password is shown in the JELOS settings.
- Ports live in `/storage/roms/ports/`. A launcher script `NetHack.sh` there
  shows up under Ports in the menu. Put the game files in
  `/storage/roms/ports/nethack/`.
- Save files, bones, record and the options file go inside that folder so
  they survive reboots.
- Andrew will give the X55's IP address. Copy builds over with scp, launch
  over ssh where useful, and read logs from the device to debug.

## Plan

```
1. SSH into the X55: record glibc version, SDL2 version, libs available
2. Download NetHack 5.0 source (nethack.org / GitHub NetHack/NetHack)
3. Set up an aarch64 build env matching the device (Docker recommended)
4. Build plain tty NetHack for aarch64 as a first sanity check
5. Write the SDL2 window port: text grid, messages, status, menus
6. Gamepad input + command menu + on-screen keyboard
7. Package NetHack.sh + nethack/ folder, deploy, test on device
8. Iterate on controls and readability with Andrew
9. Optional later: tiles; a release with source (GitHub/itch.io)
```

## Working with Andrew

- He prefers clear, organized, actionable responses.
- He'll be testing on the device, so keep each request to him concrete
  (e.g. "press Start, open the menu, tell me if X shows").
