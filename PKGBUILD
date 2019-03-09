pkgname=x-active-window-indicator
pkgver=0.0.1
pkgrel=1
pkgdesc="An X11 utility that signals the active window"
url="https://github.com/tomKPZ/x-active-window-indicator"
arch=("x86_64")
license=("GPL3")
depends=("gcc-libs" "glibc" "libxcb")
makedepends=("cmake" "git")

source=("${pkgname}::git+git://github.com/tomKPZ/x-active-window-indicator.git")
md5sums=("SKIP")

build() {
    cd "$srcdir/${pkgname}"
    cmake .
    make
}

package() {
    cd "$srcdir/${pkgname}"
    make DESTDIR="$pkgdir" PREFIX=/usr install
}
