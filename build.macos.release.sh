#!/bin/bash
mkdir obj
mkdir bin
mkdir tools/clients
rm -rf tools/clients/*
rm tools/gui/websocketd.exe
set -xe

brew reinstall wget libpng yaml-cpp freetype rapidjson mbedtls pcre libev libsodium c-ares libevent zlib

git clone https://github.com/curl/curl
cd curl
./buildconf
./configure --with-ssl=/usr/local/opt/openssl@1.1 --without-mbedtls --disable-ldap --disable-ldaps --disable-rtsp --without-libidn2
make -j8
cd ..

curl -L -o bzip2-1.0.6.tar.gz https://sourceforge.net/projects/bzip2/files/bzip2-1.0.6.tar.gz/download
tar xvf bzip2-1.0.6.tar.gz
cd bzip2-1.0.6
make -j8
cd ..

git clone https://github.com/pngwriter/pngwriter
cd pngwriter
cmake .
sudo make install -j8
cd ..

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
mv ss-local ../../tools/clients
cd ../..

git clone https://github.com/shadowsocks/simple-obfs
cd simple-obfs
git submodule init
git submodule update
./autogen.sh
CC=cc ./configure --disable-documentation
make -j8
cd src
cp /usr/local/lib/libev.a .
cc -Xlinker -unexported_symbol -Xlinker "*" obfs_local*.o ../libcork/.libs/libcork.a libev.a -o obfs-local
mv obfs-local ../../tools/clients
cd ../..

git clone https://github.com/shadowsocksrr/shadowsocksr-libev
cd shadowsocksr-libev
export LDFLAGS="-L/usr/local/opt/openssl@1.1/lib"
export CPPFLAGS="-I/usr/local/opt/openssl@1.1/include"
CC=cc ./configure --disable-documentation --with-openssl=/usr/local/opt/openssl@1.1
make -j8
cd src
cp /usr/local/lib/libpcre.a .
cp /usr/local/opt/openssl@1.1/lib/libssl.a .
cp /usr/local/opt/openssl@1.1/lib/libcrypto.a .
cp /usr/local/lib/libsodium.a .
cp /usr/local/lib/libev.a .
cc -Xlinker -unexported_symbol -Xlinker "*" ss_local*.o .libs/libshadowsocks-libev.a ../libudns/.libs/libudns.a libpcre.a libssl.a libcrypto.a libsodium.a libev.a -o ssr-local -s
mv ssr-local ../../tools/clients
cd ../..

curl -LO https://github.com/v2ray/v2ray-core/releases/latest/download/v2ray-macos.zip
unzip v2ray-macos.zip v2ray v2ctl
mv v2ray tools/clients/
mv v2ctl tools/clients/

curl -LO https://github.com/joewalnes/websocketd/releases/download/v0.3.0/websocketd-0.3.0-darwin_amd64.zip
unzip websocketd-0.3.0-darwin_amd64.zip websocketd
mv websocketd tools/gui/

cp /usr/local/lib/libevent.a .
cp /usr/local/lib/libPNGwriter.a .
cp /usr/local/lib/libpng.a .
cp /usr/local/lib/libfreetype.a .
cp /usr/local/opt/zlib/lib/libz.a .
cp /usr/local/opt/openssl@1.1/lib/libssl.a .
cp /usr/local/opt/openssl@1.1/lib/libcrypto.a .
cp /usr/local/lib/libyaml-cpp.a .

c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -I/usr/local/opt/curl/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c geoip.cpp -o obj/geoip.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -I/usr/local/opt/curl/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c logger.cpp -o obj/logger.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -I/usr/local/opt/curl/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c main.cpp -o obj/main.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -I/usr/local/opt/curl/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c misc.cpp -o obj/misc.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -I/usr/local/opt/curl/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c multithread_test.cpp -o obj/multithread_test.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -I/usr/local/opt/curl/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c printout.cpp -o obj/printout.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -I/usr/local/opt/curl/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c processes.cpp -o obj/processes.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -I/usr/local/opt/curl/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c rapidjson_extra.cpp -o obj/rapidjson_extra.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -I/usr/local/opt/curl/include -I/usr/local/include/freetype2 -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c renderer.cpp -o obj/renderer.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -I/usr/local/opt/curl/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c rulematch.cpp -o obj/rulematch.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -I/usr/local/opt/curl/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c socket.cpp -o obj/socket.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -I/usr/local/opt/curl/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c speedtestutil.cpp -o obj/speedtestutil.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -I/usr/local/opt/curl/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c tcping.cpp -o obj/tcping.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -I/usr/local/opt/curl/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c webget.cpp -o obj/webget.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -I/usr/local/opt/curl/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c webgui_wrapper.cpp -o obj/webgui_wrapper.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -I/usr/local/opt/curl/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c webserver_libevent.cpp -o obj/webserver_libevent.o
c++ -Xlinker -unexported_symbol -Xlinker "*" -o stairspeedtest obj/geoip.o obj/logger.o obj/main.o obj/misc.o obj/multithread_test.o obj/printout.o obj/processes.o obj/rapidjson_extra.o obj/renderer.o obj/rulematch.o obj/socket.o obj/speedtestutil.o obj/tcping.o obj/webget.o obj/webgui_wrapper.o obj/webserver_libevent.o libevent.a curl/lib/.libs/libcurl.a libPNGwriter.a libpng.a libfreetype.a libz.a libssl.a libcrypto.a libyaml-cpp.a bzip2-1.0.6/libbz2.a -ldl -lpthread

chmod +rx stairspeedtest pref.ini webgui.sh webserver.sh tools/clients/* tools/gui/* tools/misc/*
tar czf stairspeedtest_reborn_darwin64.tar.gz stairspeedtest pref.ini webgui.sh webserver.sh tools/

set +xe
