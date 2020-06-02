#!/bin/bash
mkdir base/tools/clients

set -xe

brew reinstall make cmake automake autoconf libtool
brew reinstall libev libsodium c-ares mbedtls

git clone https://github.com/shadowsocks/shadowsocks-libev --depth=1
cd shadowsocks-libev
git submodule update --init
./autogen.sh > /dev/null
CC=cc ./configure --disable-documentation > /dev/null
make -j8 > /dev/null
cd src
cp /usr/local/lib/libpcre.a .
cp /usr/local/lib/libmbedtls.a .
cp /usr/local/lib/libmbedcrypto.a .
cp /usr/local/lib/libev.a .
cp /usr/local/lib/libsodium.a .
cc -Xlinker -unexported_symbol -Xlinker "*" ss_local*.o .libs/libshadowsocks-libev.a libpcre.a libmbedtls.a libmbedcrypto.a libev.a libsodium.a -o ss-local
mv ss-local ../../base/tools/clients
cd ../..

git clone https://github.com/shadowsocks/simple-obfs --depth=1
cd simple-obfs
git submodule update --init
./autogen.sh > /dev/null
CC=cc ./configure --disable-documentation > /dev/null
make -j8 > /dev/null
cd src
cp /usr/local/lib/libev.a .
cc -Xlinker -unexported_symbol -Xlinker "*" obfs_local*.o ../libcork/.libs/libcork.a libev.a -o simple-obfs
mv simple-obfs ../../base/tools/clients
cd ../..

git clone -b Akkariiin/develop --single-branch --depth=1 https://github.com/shadowsocksrr/shadowsocksr-libev
cd shadowsocksr-libev
./autogen.sh > /dev/null
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

curl -LO https://github.com/trojan-gfw/trojan/releases/download/v1.15.1/trojan-1.15.1-macos.zip
unzip trojan*.zip trojan/trojan
mv trojan/trojan base/tools/clients

curl -LO https://github.com/joewalnes/websocketd/releases/download/v0.3.0/websocketd-0.3.0-darwin_amd64.zip
unzip websocketd-0.3.0-darwin_amd64.zip websocketd
mv websocketd base/tools/gui/

chmod +rx base/tools/clients/* base/tools/gui/websocketd

set +xe
