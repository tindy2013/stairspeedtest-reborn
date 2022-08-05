#!/bin/bash
mkdir base/tools/clients
export MACOSX_DEPLOYMENT_TARGET=10.13

set -xe

brew reinstall make cmake automake autoconf libtool
brew reinstall libev libsodium c-ares mbedtls@2 pcre

if [ -z "$HOMEBREW_PREFIX" ]; then
  export HOMEBREW_PREFIX=/usr/local
fi

git clone https://github.com/shadowsocks/shadowsocks-libev --depth=1
cd shadowsocks-libev
git submodule update --init
./autogen.sh > /dev/null
CC=cc ./configure --disable-documentation --with-mbedtls="$HOMEBREW_PREFIX"/opt/mbedtls@2 --with-sodium="$HOMEBREW_PREFIX"/opt/libsodium --with-cares="$HOMEBREW_PREFIX"/opt/c-ares --with-ev="$HOMEBREW_PREFIX"/opt/libev > /dev/null
make -j8 > /dev/null
cd src
cp "$HOMEBREW_PREFIX"/opt/pcre/lib/libpcre.a .
cp "$HOMEBREW_PREFIX"/opt/mbedtls@2/lib/libmbed*.a .
cp "$HOMEBREW_PREFIX"/opt/libev/lib/libev.a .
cp "$HOMEBREW_PREFIX"/opt/libsodium/lib/libsodium.a .
cc -Xlinker -unexported_symbol -Xlinker "*" ss_local*.o .libs/libshadowsocks-libev.a *.a -o ss-local
mv ss-local ../../base/tools/clients
cd ../..

git clone https://github.com/shadowsocks/simple-obfs --depth=1
cd simple-obfs
git submodule update --init
./autogen.sh > /dev/null
CC=cc ./configure --disable-documentation --with-ev="$HOMEBREW_PREFIX"/opt/libev > /dev/null
make -j8 > /dev/null
cd src
cp "$HOMEBREW_PREFIX"/opt/libev/lib/libev.a .
cc -Xlinker -unexported_symbol -Xlinker "*" obfs_local*.o ../libcork/.libs/libcork.a *.a -o simple-obfs
mv simple-obfs ../../base/tools/clients
cd ../..

git clone -b Akkariiin/develop --single-branch --depth=1 https://github.com/shadowsocksrr/shadowsocksr-libev
cd shadowsocksr-libev
./autogen.sh > /dev/null
export LDFLAGS="-L$HOMEBREW_PREFIX/opt/openssl@1.1/lib"
export CPPFLAGS="-I$HOMEBREW_PREFIX/opt/openssl@1.1/include"
CC=cc ./configure --disable-documentation --with-openssl="$HOMEBREW_PREFIX"/opt/openssl@1.1 > /dev/null
make -j8 > /dev/null
cd src
cp "$HOMEBREW_PREFIX"/opt/pcre/lib/libpcre.a ..
cp "$HOMEBREW_PREFIX"/opt/openssl@1.1/lib/libssl.a ..
cp "$HOMEBREW_PREFIX"/opt/openssl@1.1/lib/libcrypto.a ..
cp "$HOMEBREW_PREFIX"/opt/libsodium/lib/libsodium.a ..
cp "$HOMEBREW_PREFIX"/opt/libev/lib/libev.a ..
cc -Xlinker -unexported_symbol -Xlinker "*" ss_local*.o .libs/libshadowsocks-libev.a ../libudns/.libs/libudns.a ../*.a -o ssr-local
mv ssr-local ../../base/tools/clients
cd ../..

curl -LO https://github.com/v2fly/v2ray-core/releases/latest/download/v2ray-macos-64.zip
unzip v2ray*.zip v2ray v2ctl
mv v2ray v2ctl base/tools/clients/

curl -LO https://github.com/trojan-gfw/trojan/releases/download/v1.16.0/trojan-1.16.0-macos.zip
unzip trojan*.zip trojan/trojan
mv trojan/trojan base/tools/clients/

curl -LO https://github.com/joewalnes/websocketd/releases/download/v0.4.1/websocketd-0.4.1-darwin_amd64.zip
unzip websocketd-0.4.1-darwin_amd64.zip websocketd
mv websocketd base/tools/gui/

curl -LO https://github.com/shadowsocks/v2ray-plugin/releases/download/v1.3.1/v2ray-plugin-darwin-amd64-v1.3.1.tar.gz
tar xvf v2ray-plugin*.gz
rm v2ray-plugin*.gz
mv v2ray-plugin* base/tools/clients/v2ray-plugin

chmod +rx base/tools/clients/* base/tools/gui/websocketd

set +xe
