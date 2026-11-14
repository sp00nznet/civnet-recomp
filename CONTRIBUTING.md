# Contributing

Issues and pull requests are welcome.

## The one rule

**Nothing from the game goes in the repository.** No game files, no lifted C
(`work/`), no import or ordinal maps, no disassembly listings, nothing
generated from `CIVNET.EXE` or its DLLs. The tool ships; its output is made on
each person's machine from their own copy. `.gitignore` covers the usual
paths; check `git status` before you commit anyway. Screenshots of the game
running are fine.

## Before a pull request

1. Lift and build from clean: `py -3 tools\relift.py`, then
   `cmake --build build` ([docs/building.md](docs/building.md)).
2. There is no automated harness yet. Run the scripted path in
   [docs/testing.md](docs/testing.md) (`CIVNET_KEYS` input, `CIVNET_SHOT`
   captures) through to the game screen and compare the captures with
   `docs/screenshots/`. A change that moves a wall comes with a capture that
   shows it.

## Where a fix belongs

- A bug in the NE lifter, the code map or the Win16 host (a KERNEL, USER, GDI,
  WinG or MMSYSTEM call answered wrong) belongs in
  [pcrecomp](https://github.com/sp00nznet/pcrecomp), not here.
- Something only CivNet needs (its WinG lookup, its test hooks) goes in
  `runtime/civnet.c`, with a comment that says what it fixes and why.
- A wall you got past goes in `docs/bringup.md`: what it looked like, what it
  was, what moved it.
