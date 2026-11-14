# Changelog

All notable changes to this project are documented here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and the project uses
[Semantic Versioning](https://semver.org/).

## [Unreleased]

### Removed
- `analysis/win16_imports.json`. It was built from Wine's `.spec` files, which
  are LGPL, so it isn't ours to carry in an MIT repository. `tools/relift.py`
  now builds `work/win16_imports.json` on your machine the first time it runs
  (pcrecomp `tools/ne/wine_spec_to_json.py`), and it is gitignored.

### Changed
- Builds against pcrecomp main; the `pcrecomp-ne-civnet` branch is merged.
- Setup, the docs and the launcher all name the pcrecomp branch the game
  actually needs, `pcrecomp-ne-civnet`; Setup asked for
  `pcrecomp-ne-nativemap`, which lacks the resource-tag fix the map needs.
- README in the house order; `docs/lifting.md` is now `docs/bringup.md`;
  `docs/building.md` and `docs/testing.md` hold the step-by-step build and the
  scripted runs. Added NOTICE.md, CONTRIBUTING.md, CONTRIBUTORS.md, and a
  `setup.sh` that says Linux and macOS aren't supported yet.
- The lift needs no IDA: pcrecomp's `pcrecomp-ne-nativemap` branch builds the
  code map itself (`tools/ne/ne_codemap.py`). `tools/relift.py` lost `--ida`,
  `Setup.cmd` no longer checks for idalib, and it reaches the game screen as before, the captures pixel for pixel the same (38,147 functions, 2 unresolved).
- The Win16 ordinal map is built from Wine's 16-bit `.spec` files (pcrecomp
  `tools/ne/wine_spec_to_json.py`) instead of IDA's `.idt` files; every
  ordinal the game imports has the same name in both.
- In game: the map draws (pcrecomp reads CivNet's resource-tag records the
  right way round), the settlers move, and the leader-name box and the
  autosave no longer stop the first turn.

### Added
- First boot: `CIVNET.EXE`, `SMEDSNET.DLL` and `TIMERDLL.DLL` lifted together
  (38,124 functions, 2 unresolved) on pcrecomp's Win16 host, branch
  `win16-civnet`. The MicroProse intro and credits movies play full frame.
- Through the main menu, new game, world setup, tribe, leader and
  introduction into the game screen (the map is still black).
- Real child and pop-up windows, `WM_DESTROY` to the game, the screen palette;
  `CIVNET_SHOT` captures and `CIVNET_KEYS` test input (`runtime/civnet.c`).
- `tools/relift.py`, `tools/callsite_purges.py`; `docs/lifting.md`.
- `Setup.cmd` (`tools/setup.ps1`): checks and installs the tools, copies your
  CD into `game\`, lifts, builds and leaves a `CivNet.cmd` shortcut.
  `launcher/launcher.json` for recomp-launcher, and a manual-only
  `installer` workflow.
