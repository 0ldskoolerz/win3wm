# Maintainer: 0ldskoolerz <0ldskoolerz@users.noreply.github.com>
pkgname=win3wm
pkgver=0.3.0
pkgrel=1
pkgdesc="Gestor de ventanas minimalista estilo Openbox/Blackbox con estetica Windows 3.x (C + raylib + plugins Lua)"
arch=('x86_64' 'i686' 'aarch64')
url="https://github.com/0ldskoolerz/win3wm"
license=('MIT')
depends=('raylib' 'lua')
makedepends=('git')
source=("$pkgname::git+$url.git#tag=v$pkgver")  # requiere tag v0.2.1 en el repo
md5sums=('SKIP')

build() {
    cd "$pkgname"
    make
}

package() {
    cd "$pkgname"
    install -Dm755 win3wm "$pkgdir/usr/bin/win3wm"
    install -Dm644 README.md "$pkgdir/usr/share/doc/$pkgname/README.md"
    install -Dm644 docs/INSTALL.md "$pkgdir/usr/share/doc/$pkgname/INSTALL.md"
    install -Dm644 docs/PLUGINS.md "$pkgdir/usr/share/doc/$pkgname/PLUGINS.md"
    install -Dm644 docs/CONFIG.md "$pkgdir/usr/share/doc/$pkgname/CONFIG.md"
    install -Dm644 config/win3wm.conf "$pkgdir/usr/share/$pkgname/win3wm.conf"
    # plugins de ejemplo, instalados como referencia para copiar/editar
    install -dm755 "$pkgdir/usr/share/$pkgname/plugins"
    for f in plugins/*.lua plugins/README.md; do
        install -Dm644 "$f" "$pkgdir/usr/share/$pkgname/$f"
    done
}
