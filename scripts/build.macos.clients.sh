#!/bin/bash
mkdir base/tools/clients

set -xe

brew reinstall make cmake automake autoconf libtool
brew reinstall libev libsodium c-ares mbedtls

git clone https://github.com/shadowsocks/shadowsocks-libev
cd shadowsocks-libev
git submodule init
git submodule update
./autogen.sh
CC=cc ./configure --disable-documentation
make -j8
cd src
cp /usr/local/lib/libpcre.a .
cp /usr/local/lib/libmbedtls.a .
cp /usr/local/lib/libmbedcrypto.a .
cp /usr/local/lib/libev.a .
cp /usr/local/lib/libsodium.a .
cc -Xlinker -unexported_symbol -Xlinker "*" ss_local*.o .libs/libshadowsocks-libev.a libpcre.a libmbedtls.a libmbedcrypto.a libev.a libsodium.a -o ss-local
mv ss-local ../../base/tools/clients
cd ../..

git clone https://github.com/shadowsocks/simple-obfs
cd simple-obfs
git submodule init
git submodule update
./autogen.sh > /dev/null
CC=cc ./configure --disable-documentation > /dev/null
make -j8 > /dev/null
cd src
cp /usr/local/lib/libev.a .
cc -Xlinker -unexported_symbol -Xlinker "*" obfs_local*.o ../libcork/.libs/libcork.a libev.a -o simple-obfs
mv simple-obfs ../../base/tools/clients
cd ../..

git clone https://github.com/shadowsocksrr/shadowsocksr-libev
cd shadowsocksr-libev
export LDFLAGS="-L/usr/local/opt/openssl@1.1/lib"
export CPPFLAGS="-I/usr/local/opt/openssl@1.1/include"
CC=cc ./configure --disable-documentation --with-openssl=/usr/local/opt/openssl@1.1 > /dev/null
make -j8 > /dev/null
cd src
cp /usr/local/lib/libpcre.a .
cp /usr/local/opt/openssl@1.1/lib/libssl.a .
cp /usr/local/opt/openssl@1.1/lib/libcrypto.a .
cp /usr/local/lib/libsodium.a .
cp /usr/local/lib/libev.a .
cc -Xlinker -unexported_symbol -Xlinker "*" ss_local*.o .libs/libshadowsocks-libev.a ../libudns/.libs/libudns.a libpcre.a libssl.a libcrypto.a libsodium.a libev.a -o ssr-local
mv ssr-local ../../base/tools/clients
cd ../..

curl -LO https://github.com/v2ray/v2ray-core/releases/latest/download/v2ray-macos.zip
unzip v2ray-macos.zip v2ray v2ctl
mv v2ray base/tools/clients
mv v2ctl base/tools/clients

curl -LO https://github.com/joewalnes/websocketd/releases/download/v0.3.0/websocketd-0.3.0-darwin_amd64.zip
unzip websocketd-0.3.0-darwin_amd64.zip websocketd
mv websocketd base/tools/gui/

chmod +rx base/tools/clients/* base/tools/gui/websocketd

set +xe
