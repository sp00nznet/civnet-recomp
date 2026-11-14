"""
relift.py - Regenerate CivNet's lifted tree from your own copy of the game.

Everything below work/ is derived from CIVNET.EXE and its DLLs and is never
committed. One command, so a re-lift cannot silently skip a step:

    python tools/relift.py

No IDA: pcrecomp builds each module's code map itself (tools/ne/ne_codemap.py,
the default inside ne_link).

What it runs, all from pcrecomp (PCRECOMP_DIR, default ../pcrecomp, branch main):
  ne/ne_link.py       lift both modules into one numbering, build the image,
                      and regenerate the glue (segments.h, dispatch, stubs)
  ne/patch_borland_eh.py  the Borland EH cleanup-chain guard
  ne/wine_spec_to_json.py the Win16 ordinal map (work/win16_imports.json), once,
                      from Wine's .spec files (downloaded; LGPL data, so it is
                      built here and never committed: NOTICE.md)
"""
import os
import subprocess
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))
PC = os.path.abspath(os.environ.get('PCRECOMP_DIR', os.path.join(ROOT, '..', 'pcrecomp')))
MODULES = ['game/SMEDSNET.DLL', 'game/TIMERDLL.DLL', 'game/CIVNET.EXE']


def run(*cmd):
    print('>', ' '.join(cmd), flush=True)
    subprocess.run(cmd, cwd=ROOT, check=True)


def main():
    imports = os.path.join(ROOT, 'work', 'win16_imports.json')
    if not os.path.exists(imports):
        os.makedirs(os.path.dirname(imports), exist_ok=True)
        run(sys.executable, os.path.join(PC, 'tools', 'ne', 'wine_spec_to_json.py'), '-o', imports)
    run(sys.executable, os.path.join(PC, 'tools', 'ne', 'ne_link.py'),
        '--work', 'work',
        '--prefix', 'recomp',
        '--shims', os.path.join(PC, 'runtime', 'win16', 'host'), '--shims', 'runtime',
        *MODULES)
    run(sys.executable, os.path.join(PC, 'tools', 'ne', 'patch_borland_eh.py'),
        '--work', 'work', *MODULES)


if __name__ == '__main__':
    main()
