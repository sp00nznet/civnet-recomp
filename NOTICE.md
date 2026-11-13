# Notice

The MIT license in [LICENSE](LICENSE) covers what is ours in this repository:
the runtime glue in `runtime/`, the setup scripts, the tools and the
documentation. It does not cover the game.

*Sid Meier's CivNet* is © 1995 MicroProse, and is not ours to license. Its
executables, DLLs, data, movies and sounds are not in this repository and are
excluded by `.gitignore`.

- You need your own copy of the game to use any of this. Setup copies your CD
  into `game/`; the pipeline reads it from there.
- Lifted code is a derivative work of the binary it came from. Anything the
  pipeline writes into `work/` or `src/recomp/gen*/` carries whatever license
  the original carries, not this one. The MIT grant stops at our own work.
- The Win16 host and the NE lifter are [pcrecomp](https://github.com/sp00nznet/pcrecomp)'s
  (MIT), fetched beside this repository, not copied into it.

## Third-party data used at build time, not in this repository

| What | Where it lands | Upstream | License |
|---|---|---|---|
| Win16 API names by ordinal | `work/win16_imports.json` (gitignored) | Wine's 16-bit `.spec` files, downloaded by pcrecomp `tools/ne/wine_spec_to_json.py` | LGPL-2.1-or-later (Wine) |

The map is built on your machine when you lift and is never committed, so no
Wine material is distributed from here.
