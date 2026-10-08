# Changelog

## [0.3.0] — Addons de sistema: audio, red, bluetooth, discos

### Añadido
- `audio.lua`: volumen y mute reales vía `pactl`; F8/F9/F10.
- `red.lua`: conexión ethernet/wifi vía `nmcli`; F11 lista redes.
- `bluetooth.lua`: estado y escaneo vía `bluetoothctl`; F7.
- `discos.lua`: uso de discos vía `df`/`lsblk`; F5 lista montables,
  F6 desmonta lo montado bajo /media y /mnt.
- Documentación de los hotkeys y dependencias en docs/PLUGINS.md.

## [0.2.1] — Hotkeys expuestos a Lua + PKGBUILD

### Añadido
- Evento `on_key(code, nombre)` en plugins: cada pulsación de tecla llega a
  los plugins con el código raylib y un nombre legible (`F2`, `a`, `ESC`,
  `UP`, ...). Ver docs/PLUGINS.md.
- Plugin `keys.lua` de ejemplo (muestra la tecla pulsada en el panel).
- `launcher.lua` ahora abre ventanas realmente con F2 vía `on_key`.
- `PKGBUILD` para instalar como paquete pacman en Arch
  (`makepkg -si`, requiere el tag v0.2.1).
- Soporte de `GetKeyPressed()` en el stub de tests.

## [0.2.0] — Panel configurable estilo lxpanel

### Añadido
- Panel de estado modular: clave `panel_modules` en la configuración define
  las secciones en orden (`tasks`, `clock`, `date` nativas + slots libres).
- API Lua nueva: `wm.status(modulo, texto)` para escribir secciones del panel,
  `wm.window_count()` y `wm.window_ids()`.
- Plugins nuevos: `mem.lua` (uso de RAM desde /proc/meminfo) y `apps.lua`
  (lista de apps corriendo).
- Documentación completa: `docs/INSTALL.md`, `docs/PLUGINS.md`,
  `docs/CONFIG.md`.
- Soporte de pkg-config para `lua.pc` (nombres de Arch, además de Debian).

## [0.1.0] — MVP inicial

### Añadido
- Núcleo del WM en C puro (sin dependencias): crear/cerrar/enfocar ventanas,
  orden Z, mover, redimensionar por bordes/esquinas, minimizar/maximizar,
  colocación en cascada estilo Win 3.x, hit-testing completo.
- Frontend raylib: decoración Win 3.x (biselados, barra de título navy),
  barra de tareas, menú del sistema, Alt+Tab, F1.
- Host de plugins Lua con estados aislados y degradación limpia sin Lua.
- API Lua: `wm.log/notify/create_window/close_window/focus/focus_next/
  window_title/set_title` y eventos `on_start/on_tick/on_stop/
  on_window_focused/on_window_closed`.
- Configuración rc-file (`config/win3wm.conf`) con temas por nombre de color.
- Plugins de ejemplo: `clock.lua`, `greeter.lua`, `launcher.lua`.
- Tests del core: 35 checks (`make test`).
