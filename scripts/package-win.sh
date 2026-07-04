#!/usr/bin/env bash
# 一键打包 Windows 单文件版 wiliwili
#   - 无黑框（Windows GUI 子系统）
#   - mpv + ffmpeg + 资源 全部内嵌
#   - curl / webp 静态链接 → 零外部 DLL 依赖
# 产物是单个 exe，拷到任何 Windows 电脑双击即可运行（和官方 release 一致）。
#
# 用法（在 MSYS2 MinGW64 shell 或 git bash 里）：
#     bash scripts/package-win.sh
#
# 依赖：MSYS2 mingw64 里装好基础工具链
#     pacman -S mingw-w64-x86_64-{gcc,cmake,ninja,mpv,zlib}
#   （webp/curl 本脚本会自己编静态库，不用系统的）

set -euo pipefail

# ---- 可按需修改 ----
MINGW_PREFIX="${MINGW_PREFIX:-/e/Tools/msys/mingw64}"   # 你的 MSYS2 mingw64 路径
BUILD_DIR="build-win"
MPV_PKG_URL="https://github.com/xfangfang/wiliwili/releases/download/v0.1.0/mingw-w64-x86_64-mpv-0.40.0-2-any.pkg.tar.zst"
WEBP_VER="1.6.0"
CURL_VER="8.7.1"
# --------------------

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
export PATH="$MINGW_PREFIX/bin:$PATH"
DEPS="$ROOT/win-deps"           # 静态 curl/webp 装到这里，不污染系统 MSYS2

echo ">>> MinGW: $MINGW_PREFIX"
command -v gcc   >/dev/null || { echo "找不到 gcc，请检查 MINGW_PREFIX"; exit 1; }
command -v ninja >/dev/null || { echo "找不到 ninja: pacman -S mingw-w64-x86_64-ninja"; exit 1; }

# 1) 自包含 mpv（ffmpeg 静态编入，~34M）。系统默认 mpv 是空壳，别的电脑放不了视频。
BUNDLE_MPV="$ROOT/custom-mpv/mingw64/bin/libmpv-2.dll"
if [ ! -f "$BUNDLE_MPV" ]; then
    echo ">>> 下载自包含 mpv ..."
    mkdir -p custom-mpv && ( cd custom-mpv \
        && curl -L --retry 3 -o mpv.pkg.tar.zst "$MPV_PKG_URL" \
        && tar --use-compress-program=unzstd -xf mpv.pkg.tar.zst )
fi
echo ">>> 内嵌 mpv: $(du -h "$BUNDLE_MPV" | cut -f1)"

# 2) 静态 webp（含 sharpyuv），装进本地 prefix
if [ ! -f "$DEPS/lib/libwebp.a" ]; then
    echo ">>> 编译静态 libwebp $WEBP_VER ..."
    [ -d "/tmp/libwebp-$WEBP_VER" ] || curl -sL "https://github.com/webmproject/libwebp/archive/v$WEBP_VER.tar.gz" | tar zxf - -C /tmp
    cmake -B _dep-src/build-webp -S "/tmp/libwebp-$WEBP_VER" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$DEPS" \
        -DBUILD_SHARED_LIBS=OFF \
        -DWEBP_BUILD_ANIM_UTILS=OFF -DWEBP_BUILD_CWEBP=OFF -DWEBP_BUILD_DWEBP=OFF \
        -DWEBP_BUILD_GIF2WEBP=OFF -DWEBP_BUILD_IMG2WEBP=OFF -DWEBP_BUILD_VWEBP=OFF \
        -DWEBP_BUILD_WEBPINFO=OFF -DWEBP_BUILD_WEBPMUX=OFF -DWEBP_BUILD_LIBWEBPMUX=OFF \
        -DWEBP_BUILD_EXTRAS=OFF
    cmake --build _dep-src/build-webp
    cmake --install _dep-src/build-webp
fi

# 3) 静态 curl（Schannel，禁掉会拖 DLL 的 brotli/idn2/nghttp2/zstd，zlib 走静态）
if [ ! -f "$DEPS/lib/libcurl.a" ]; then
    echo ">>> 编译静态 libcurl $CURL_VER ..."
    [ -d "/tmp/curl-$CURL_VER" ] || curl -sL "https://curl.se/download/curl-$CURL_VER.tar.xz" | tar Jxf - -C /tmp
    cmake -B _dep-src/build-curl -S "/tmp/curl-$CURL_VER" -G Ninja \
        -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX="$DEPS" \
        -DBUILD_SHARED_LIBS=OFF -DHTTP_ONLY=ON -DCURL_USE_SCHANNEL=ON \
        -DCURL_DISABLE_PROGRESS_METER=ON -DBUILD_CURL_EXE=OFF -DBUILD_TESTING=OFF \
        -DBUILD_LIBCURL_DOCS=OFF -DUSE_LIBIDN2=OFF -DUSE_WIN32_IDN=ON \
        -DCURL_USE_LIBSSH2=OFF -DCURL_USE_LIBPSL=OFF \
        -DUSE_NGHTTP2=OFF -DCURL_BROTLI=OFF -DCURL_ZSTD=OFF \
        -DZLIB_USE_STATIC_LIBS=ON
    cmake --build _dep-src/build-curl
    cmake --install _dep-src/build-curl
fi

# 4) 配置 wiliwili（CMAKE_PREFIX_PATH 指向 win-deps → 优先用静态 curl/webp）
cmake -B "$BUILD_DIR" -G Ninja \
    -DCMAKE_PREFIX_PATH="$DEPS" \
    -DPLATFORM_DESKTOP=ON \
    -DWIN32_TERMINAL=OFF \
    -DUSE_SYSTEM_CURL=ON \
    -DUSE_LIBROMFS=ON \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DMPV_BUNDLE_DLL="$BUNDLE_MPV"

# 5) 编译
cmake --build "$BUILD_DIR"

# 6) strip 出成品（单文件，无需附带任何 DLL）
DIST="$BUILD_DIR/dist"
rm -rf "$DIST" && mkdir -p "$DIST"
cp "$BUILD_DIR/wiliwili.exe" "$DIST/"
strip "$DIST/wiliwili.exe"

echo ""
echo ">>> 完成: $ROOT/$DIST/wiliwili.exe ($(du -h "$DIST/wiliwili.exe" | cut -f1))"
echo ">>> 单文件、零外部 DLL，拷到任意 Windows 电脑双击即可运行。"
