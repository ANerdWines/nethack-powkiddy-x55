# SDL2 Window Port Implementation Plan

Target: NetHack 5.0 on Powkiddy X55 (JELOS) with gamepad controls.

## Phases

- [x] **Phase 1: Basic Rendering** (COMPLETE)
  - Create `win/sdl2/` directory with `winsdl2.h` and `winsdl2.c`
  - Data structures: `SDL2_WinDesc`, `SDL2_DisplayDesc`, map buffer
  - SDL2 init with KMSDRM fullscreen, load DejaVu Sans Mono font
  - Implement: `init_nhwindows`, `exit_nhwindows`, `create/destroy/clear/display_nhwindow`
  - Implement: `putstr`, `raw_print`, `print_glyph`, `curs`
  - Stub remaining functions
  - Build integration: hints files, Makefile rules
  - **Test:** Game launches, shows map and text

- [ ] **Phase 2: Input**
  - Implement `sdl2_nhgetch()` with gamepad support
  - D-pad/stick direction mapping (8-way)
  - Button mapping: A=confirm, B=ESC, X=pickup, Y=search, Select=inventory
  - Implement `sdl2_yn_function()` for yes/no prompts
  - Input repeat for movement
  - **Test:** Can move player, answer prompts, open inventory

- [ ] **Phase 3: Menus**
  - Implement menu system: `start_menu`, `add_menu`, `end_menu`, `select_menu`
  - Menu rendering with pagination
  - L1/R1 for page up/down
  - **Test:** Inventory works, can eat food, pick up items

- [ ] **Phase 4: Text Input & Status**
  - On-screen keyboard for `sdl2_getlin()`
  - Status line rendering: `status_init`, `status_update`
  - Implement `sdl2_askname()` using OSK
  - **Test:** Can enter player name, status shows HP/stats

- [ ] **Phase 5: Polish**
  - Command menu (Start button) with all commands
  - Message history (`doprev_message`)
  - Extended command support (`get_ext_cmd`)
  - Player selection flow
  - **Test:** Full game session from start to dungeon exploration

## Progress Log

### Phase 1 Complete
- SDL2 window port created with basic rendering
- Cross-compilation working for aarch64
- Deployed to X55 at /storage/roms/ports/nethack/
- Launcher script at /storage/roms/ports/NetHack.sh
- Ready for testing: launch via JELOS Ports menu
- Known limitations: text input not working (Phase 4), menus need polish (Phase 3)

---

## File Structure

```
win/sdl2/
├── winsdl2.h       - data structures, declarations
├── winsdl2.c       - window_procs, init, core functions
├── sdl2map.c       - map rendering (Phase 1)
├── sdl2menu.c      - menu system (Phase 3)
├── sdl2input.c     - gamepad, OSK (Phase 2, 4)
└── sdl2status.c    - status lines (Phase 4)
```

## Control Mapping

| Input | Action |
|-------|--------|
| D-pad/Stick | Move (8 directions) |
| A | Confirm / Rest (`.`) |
| B | Cancel (ESC) |
| X | Pick up (`,`) |
| Y | Search (`s`) |
| L1/R1 | Page Up/Down |
| Start | Command Menu |
| Select | Inventory (`i`) |
