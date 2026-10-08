# win3wm

Un gestor de ventanas minimalista inspirado en **Openbox/Blackbox**, con la
estética y comportamiento de **Windows 3.x**, construido en **C + raylib** y con
**plugins scriptables en Lua**.

![estado](https://img.shields.io/badge/estado-MVP-green)

## Características

- Decoración de ventanas estilo Win 3.x: barra de título, botones
  maximizar / minimizar / cerrar, bordes biselados.
- Mover ventanas arrastrando la barra de título.
- Redimensionar arrastrando bordes y esquinas.
- Enfoque por clic + Alt+Tab para ciclar ventanas.
- **Panel de estado configurable** (estilo lxpanel/tint2): secciones en orden
  definidas en `panel_modules` — apps corriendo (`tasks`), reloj (`clock`),
  fecha (`date`), y slots libres que los plugins llenan vía `wm.status()`
  (incluye `mem.lua` para uso de RAM y `apps.lua` para lista de apps).
- Menú del sistema (Nuevo / Cascade / Minimizar todo).
- Sistema de configuración tipo rc-file (`config/win3wm.conf`).
- Plugins en Lua, cada uno en su propio estado aislado.

## Documentación

| Documento | Contenido |
|---|---|
| [docs/INSTALL.md](docs/INSTALL.md) | Compilación, instalación en Arch (con y sin entorno gráfico), headless con Xvfb, solución de problemas |
| [docs/PLUGINS.md](docs/PLUGINS.md) | Guía completa de plugins: API `wm.*`, eventos, panel, ejemplos paso a paso |
| [docs/CONFIG.md](docs/CONFIG.md) | Referencia de configuración: todas las claves, temas, recetas |
| [plugins/README.md](plugins/README.md) | Resumen rápido de la API de plugins |

## Compilar y ejecutar

```sh
# Arch Linux:
sudo pacman -S base-devel raylib lua
make
./win3wm config/win3wm.conf
```

Sin raylib instalado puedes verificar la lógica del core:

```sh
make test     # 35 checks
```

Ver [docs/INSTALL.md](docs/INSTALL.md) para entornos mínimos/headless.

## Controles

| Acción                    | Control                              |
|---------------------------|--------------------------------------|
| Crear ventana             | Menú → Nuevo, o F1                   |
| Mover ventana             | Arrastrar barra de título            |
| Redimensionar             | Arrastrar borde/esquina              |
| Enfoque                   | Clic en la ventana                   |
| Alt-Tab                   | Alt+Tab                              |
| Minimizar / Maximizar     | Botones `_` / `^` en la barra de título |
| Cerrar                    | Botón `x`                            |
| Menú del sistema          | Clic en "win3wm" en la barra de tareas |

## Estructura

```
src/
  wm.h / wm.c       núcleo del gestor de ventanas (sin dependencias)
  config.h / .c     parser de configuración key = value
  plugins.h / .c    host de plugins Lua + API hacia el core
  main.c            frontend raylib: render Win 3.x, input, taskbar, panel
plugins/
  clock.lua         reloj y fecha en el panel de estado
  greeter.lua       demostración de eventos
  mem.lua           uso de RAM (lee /proc/meminfo)
  apps.lua          lista de apps corriendo
  launcher.lua      ejemplo de creación de ventanas desde Lua
config/
  win3wm.conf       configuración por defecto
docs/
  INSTALL.md        compilación e instalación (incl. headless)
  PLUGINS.md        guía de desarrollo de plugins
  CONFIG.md         referencia de configuración
tests/
  test_wm.c         tests del core (35 checks)
  run_tests.sh
```

## Requisitos

- gcc o clang (C11), make, pkgconf
- [raylib](https://www.raylib.com/) >= 4.0
- Lua 5.3/5.4 (opcional: sin Lua compila y avisa)

## Licencia

MIT (o la que prefieras al publicarlo).
