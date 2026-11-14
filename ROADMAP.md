# Roadmap

## Done

- The lift: `CIVNET.EXE`, `SMEDSNET.DLL` and `TIMERDLL.DLL` in one tree,
  38,147 functions, 2 unresolved, with no IDA.
- From the intro movies through every setup screen to the first turn, the map
  drawn and the settlers moving.
- Setup.cmd, from the ZIP download to a shortcut.

## Next
- Labels in the game's own list and edit controls; typing a leader name.
- ShowScrollBar, GetScrollRange, WIN87EM `__fpmath`.
- Play on: found a city, end turns (the settlers already move).

## Then
- Sound and music; then multiplayer over SMEDSNET.
- A conformance harness: a fixed list of milestones (boot, intro, menus, the
  game screen) checked by `CIVNET_SHOT` captures, with a pass count in the
  README and a netlab `QA` line.
- Headless mode, `--headless --record out.mp4`, so runs and captures need no
  screen (REPO_RULES section 10).
- Linux and macOS: a `setup.sh` that cross-compiles the Windows exe for Wine
  (the host is Win32, so a native build waits on pcrecomp's win16 host
  running on win32hle).
- A published installer (recomp-launcher) once the game is playable.

## Out of scope
- Shipping any part of the game, or the lifted C.
