# win3wm — Guía de compilación e instalación

Esta guía cubre desde un sistema Arch Linux recién instalado (sin entorno
gráfico) hasta tener win3wm compilado y corriendo.

---

## 1. Requisitos

### Obligatorios para compilar

| Paquete | Propósito | Tamaño aprox. |
|---|---|---|
| `base-devel` | gcc, make, pkgconf | ~50 MB |
| `raylib` | render 2D/OpenGL (≥ 4.0) | ~5 MB |
| `lua` | intérprete embebido para plugins (5.3/5.4) | ~1 MB |

### Opcionales según escenario

| Escenario | Paquetes extra |
|---|---|
| Ejecutar en escritorio normal (X11/Wayland) | ninguno |
| Ejecutar headless (servidor sin monitor, CI) | `xorg-server-xvfb` |
| Verlo remotamente por SSH | `xorg-xauth` (X-forwarding) o `x11vnc` (VNC) |

### NO necesitas

- Un WM de X11 real: win3wm es un simulador autónomo; convive con i3,
  GNOME, KDE, sway, etc. sin tocarlos.
- Drivers de GPU: el render 2D funciona con OpenGL por software (llvmpipe).
- Root, CMake, Python, luarocks, LuaJIT.

---

## 2. Instalación en Arch Linux (escritorio con gráficos)

```sh
sudo pacman -S base-devel raylib lua
```

Descarga el proyecto (desde tu Drive o el repo de GitHub) y entra al
directorio raíz (donde está el `Makefile`):

```sh
cd win3wm
make
./win3wm config/win3wm.conf
```

Salida esperada: escritorio teal estilo Win 3.x, barra de tareas abajo,
ventana de prueba, reloj/fecha/RAM/apps en el panel de estado.

---

## 3. Instalación en un sistema SIN entorno gráfico (headless)

Un servidor o VM sin monitor no tiene display, pero win3wm puede correr
sobre un servidor X virtual:

```sh
sudo pacman -S base-devel raylib lua xorg-server-xvfb

# compilar (funciona sin display)
make

# crear un display virtual de 1024x768
Xvfb :99 -screen 0 1024x768x24 &
export DISPLAY=:99

# ejecutar
./win3wm config/win3wm.conf &
```

### 3.1 Verificar que corre (sin verlo)

```sh
# si el proceso sigue vivo tras 10 segundos, no crasheó
sleep 10 && kill -0 $(pgrep win3wm) && echo "OK: corriendo"
```

### 3.2 Verlo remotamente (opcional)

**Opción A — X forwarding (simple, lento):**

```sh
# en tu máquina local:
ssh -X servidor
./win3wm config/win3wm.conf
```

**Opción B — VNC por túnel SSH (solo localhost, no expuesto):**

```sh
# en el servidor, con Xvfb :99 ya corriendo y win3wm lanzado:
sudo pacman -S x11vnc
x11vnc -display :99 -localhost -nopw &

# en tu máquina local:
ssh -L 5900:localhost:5900 servidor
# luego conecta tu cliente VNC a localhost:5900
```

⚠️ Nunca expongas el VNC a 0.0.0.0 o uses `-nopw` sin túnel SSH.

---

## 4. Objetivos del Makefile

| Comando | Qué hace |
|---|---|
| `make` | Compila el binario completo (raylib + Lua) → `./win3wm` |
| `make win3wm-core` | Solo la lógica del WM, sin raylib ni Lua (verificación) |
| `make test` | Compila y corre los tests del core (35 checks) |
| `make clean` | Borra binarios y artefactos de tests |

### Detección de dependencias

El Makefile usa `pkg-config` para encontrar raylib y Lua. Prueba en orden:
`lua5.4` → `lua5.3` → `lua` (este último es el nombre en Arch).

Comprueba tu entorno con:

```sh
pkg-config --cflags raylib   # debe imprimir algo, no error
pkg-config --cflags lua      # idem
```

### Compilación sin Lua

Si no tienes Lua, puedes compilar de todos modos omitiendo los plugins:

```sh
gcc -O2 -Wall -std=c11 -o win3wm src/main.c src/wm.c src/config.c src/plugins.c \
    $(pkg-config --cflags --libs raylib) -lm
```

Al arrancar verás en stderr:
`[plugins] 'clock' skipped: built without Lua (-DWM_WITH_LUA)`.
Todo lo demás funciona igual.

---

## 5. Instalación en el sistema (opcional)

No hay `make install` deliberadamente (estilo suckless). Opciones:

**A. Binario en tu PATH (recomendado):**

```sh
mkdir -p ~/.local/bin
cp win3wm ~/.local/bin/
# win3wm busca plugins/ y config/ relativos al CWD:
# crea enlaces o copia las carpetas donde lo vayas a ejecutar
```

**B. PKGBUILD (paquete pacman):** pendiente de escribir; pídelo si lo quieres.

---

## 6. Solución de problemas

| Síntoma | Causa probable | Solución |
|---|---|---|
| `fatal error: raylib.h` | raylib no instalado o pkgconf no lo ve | `sudo pacman -S raylib pkgconf` |
| `[plugins] ... skipped: built without Lua` | compilaste sin `-DWM_WITH_LUA` o falta lua | instala `lua` y rehaz `make clean && make` |
| No abre ventana en Wayland | sesión Wayland sin Xwayland | usa Xwayland o sesión X11 |
| `cannot open display` | no hay DISPLAY | exporta `DISPLAY=:99` con Xvfb corriendo |
| Ventana negra / lenta | OpenGL por software en VM | normal en VMs; usa `Xvfb` con 24bpp |
| Plugins no aparecen | ejecutaste desde otro directorio | corre desde la raíz del proyecto, o ajusta rutas |
