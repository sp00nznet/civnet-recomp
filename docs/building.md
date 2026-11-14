# Building by hand

`Setup.cmd` does all of this for you; this is what it does, command for
command.

## Prerequisites

Windows 10/11, **Python 3.10+** (`py -3 --version`) with `capstone`, **CMake**
3.16+, **Ninja**, **MSYS2** with `mingw-w64-x86_64-gcc`
(`C:\msys64\mingw64\bin\gcc.exe`), **git**, and the pcrecomp toolkit cloned
**beside** this repository as `pcrecomp`, on `main` (or
point `PCRECOMP_DIR` at it). The lifter and the runtime must come from the
same tree.

```
some-folder\
  pcrecomp\        <- git clone https://github.com/sp00nznet/pcrecomp
  civnet-recomp\   <- this repository
```

## Steps

1. `py -3 -m pip install --user capstone`
2. Copy the CD's root into `game\`, leaving out `INTERNET\` and `CIVGUIDE\`:
   `robocopy E:\ game /E /XD E:\INTERNET E:\CIVGUIDE`
3. Lift into `work\`: `py -3 tools\relift.py`. The first run builds
   `work\win16_imports.json` from Wine's `.spec` files (pcrecomp
   `tools/ne/wine_spec_to_json.py`, a download of a few hundred KB); then
   `ne_link` lifts the three modules into one tree, with pcrecomp's own code
   map (no IDA), and `patch_borland_eh` guards the Borland EH chains.
4. Build:
   ```
   cmake -S . -B build -G Ninja -DCMAKE_C_COMPILER=C:/msys64/mingw64/bin/gcc.exe
   cmake --build build
   ```
   `-DPCRECOMP_DIR=` points at a toolkit other than `..\pcrecomp`. The build
   is `-O2` on purpose: see the comment in [CMakeLists.txt](../CMakeLists.txt).
5. Run `build\civnet.exe`.

On the lab, `netlab build civnet-recomp` builds it on the MinGW builder.

## Trip-ups

- `python` opening the Microsoft Store: that's Windows' alias placeholder; use `py -3`.
- A freshly installed tool isn't on `PATH` until you open a new window.
- gcc failing to find `cc1`: `C:\msys64\mingw64\bin` isn't on `PATH`.
- An older pcrecomp (before `pcrecomp-ne-civnet` was merged into `main`)
  lacks this game's resource-tag fix, and the map draws black.
- No network on the first lift: `wine_spec_to_json.py --specs DIR -o work\win16_imports.json`
  reads the `.spec` files from a local Wine source tree instead.
