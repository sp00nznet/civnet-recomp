# Bringing up CivNet: every wall so far

The layout is Space Bucks': `ne_link` lifts CIVNET.EXE with SMEDSNET.DLL (the
network layer it imports) and TIMERDLL.DLL (which it loads, for its timer
callback) into one tree, with pcrecomp's own code map (an IDA one until
`ne_codemap.py` matched it). The nine data DLLs
(CIVDATA0-6, CIVENG0-1) only carry resources and are loaded as they are.
Nearly all of this went into pcrecomp's Win16 host (PRs #179 and up).

1. **Names and purges.** Wine's 16-bit `.spec` files name the imports
   (`work/win16_imports.json`, built at lift time by pcrecomp's
   `wine_spec_to_json.py` and never committed; it began as IDA's `.idt` files,
   which agree on every ordinal CivNet imports); 36 more purges came from the Windows 3.1
   prototypes (comm ports, carets, ToolHelp, a few GDI/USER).
2. **WinG by GetProcAddress.** The game loads WING.DLL itself; the host's WinG
   shims are handed out as host thunks (`runtime/civnet.c`).
3. **The multimedia timer.** `timeSetEvent` was missing (ERR_TIMERSETFAILED),
   and the movie player waits for its callback without pumping messages, as an
   interrupt would allow. Callbacks are delivered from `recomp_tick`
   (RECOMP_IRQ: lifted function entries and loop back-edges).
4. **Resource tags.** Like PF Magic's games, the data asks for resources by a
   4-letter tag ("STRG", "SPRT") that records map to numeric types. CivNet's
   records are in DLLs it LoadLibrary's, and each DLL numbers its types from
   0x7F01, so tags resolve per module. The host also ran out of resource
   handles at 512.
5. **Windows.** Real child and pop-up windows with their own geometry
   (`real_windows`): the movie was drawn at 0,0 and over the menu bar.
   `WM_DESTROY` reaches the game (`destroy_message`), which unlinks a window
   from its parent there; without it a parent's child list became a loop.
   `GetClassWord(GCW_CBWNDEXTRA)` answered 0, and a subclassed control called
   itself until the host stack ran out.
6. **32-bit offsets.** The movie blit writes `es:[edi]` past 64 KB of its WinG
   surface; the 16-bit memory accessors cut the offset to 16 bits, and only the
   top 128 rows showed. The lifter marks those accesses (ADDR32).
7. **Text.** `EnumFontFamilies` (the game looks for its font, CIVFONTS.FON),
   `GetTextExtentPoint` (unanswered, a dialog came out 8,619 pixels wide),
   `ExtTextOut`, `GetTextAlign`; and `GetFreeSystemResources`, whose 0 brought
   up a "low memory" warning at every start.
8. **The map.** It opened black: every terrain and unit sprite is cut from one
   sheet, CvPc 2001 in CIVDATA4, and the host served CIVDATA0's #137 instead.
   An integer resource id came with an empty name string, which sent the
   lookup into the DLL's "name table" (really its tag records). The records
   themselves are [00 00][0C 00][type][id][tag], type and id before the tag
   (`resource_tags = 2`). Found by following the tile renderer (seg021 DBF6)
   to the sprite objects, whose data pointers the sheet cutter left NULL.
9. **Leader name and autosave.** DefWindowProc got 16:16 pointers as flat ones
   (a crash in the leader-name box); OpenFile ignored OF_CREATE, so the
   autosave failed and its error box blocked the first turn.

## Next

- Text inside the game's own list and edit controls (tribe and leader names).
- ShowScrollBar, GetScrollRange; WIN87EM's `__fpmath`.
