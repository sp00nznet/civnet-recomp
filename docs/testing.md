# Testing

There is no automated harness yet ([ROADMAP.md](../ROADMAP.md)). Runs are
scripted with two environment variables read by `runtime/civnet.c`, and
checked by eye against `docs/screenshots/`.

```
set CIVNET_KEYS=30:enter,35:item:2,40:click:320:200   # input to the game's own windows, at S seconds
set CIVNET_SHOT=20:menu.bmp,60:setup.bmp              # save every window of the game at 20 s and 60 s
build\civnet.exe
```

`CIVNET_KEYS` takes `S:enter`, `S:esc`, `S:space`, `S:tab`, `S:up`, `S:down`, `S:left`, `S:right`, `S:item:N`, `S:pop:x:y`, `S:click:x:y` (client
coordinates of the main window), `S:quit`, `S:ring` (print the recent lifted
calls), `S:peek:SEG:OFF:N`, and `S:snap` (with
`RECOMP_DUMP_BLIT=snap99999999`, every WinG surface to `RECOMP_DUMP_DIR`).

Enter at each prompt walks from the intro through the menus to the game
screen; arrow keys move the settlers. The game opens a window, so run it
muted on a virtual monitor (offstage) rather than on a real screen.
