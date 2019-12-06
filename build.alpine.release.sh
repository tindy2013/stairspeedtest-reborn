#!/bin/bash
mkdir obj
mkdir bin
mkdir tools/clients
rm -rf tools/clients/*
rm tools/gui/websocketd.exe
set -xe

apk add gcc g++ cmake make autoconf automake libtool build-base linux-headers
apk add libpng-dev libressl-dev freetype-dev freetype-static zlib-dev rapidjson-dev mbedtls-dev mbedtls-static libev-dev pcre-dev libsodium-dev c-ares-dev

git clone https://github.com/curl/curl
cd curl
./buildconf
./configure
make install -j4
cd ..

git clone https://github.com/jbeder/yaml-cpp
cd yaml-cpp
cmake .
make install -j4
cd ..

git clone https://github.com/pngwriter/pngwriter
cd pngwriter
cmake .
make install -j4
cd ..

git clone https://github.com/shadowsocks/simple-obfs
cd simple-obfs
git submodule init
git submodule update
./autogen.sh
./configure --disable-documentation
make -j4
cd src
gcc obfs_local*.o ../libcork/.libs/libcork.a -o simple-obfs -lev -s -static
mv simple-obfs ../../tools/clients/
cd ../..

git clone https://github.com/shadowsocks/shadowsocks-libev
cd shadowsocks-libev
git submodule init
git submodule update
./autogen.sh
./configure --disable-documentation
make -j4
cd src
gcc ss_local*.o .libs/libshadowsocks-libev.a -o ss-local -lpcre -lmbedtls -lmbedcrypto -lev -lsodium -s -static
mv ss-local ../../tools/clients/
cd ../..

git clone https://github.com/shadowsocksrr/shadowsocksr-libev
cd shadowsocksr-libev
git checkout -b latest origin/Akkariiin/develop
./autogen.sh
./configure --disable-documentation
make -j4
cd src
gcc ss_local*.o .libs/libshadowsocks-libev.a ../libudns/.libs/libudns.a -o ssr-local -lpcre -lssl -lcrypto -lev -lsodium -s -static
mv ssr-local ../../tools/clients/
cd ../..

git clone git://sourceware.org/git/bzip2.git
cd bzip2
make install -j4
cd ..

apk del wget
apk add wget

wget https://github.com/joewalnes/websocketd/releases/download/v0.3.0/websocketd-0.3.0-linux_amd64.zip
unzip websocketd-0.3.0-linux_amd64.zip websocketd
mv websocketd tools/gui/

wget https://github.com/v2ray/v2ray-core/releases/latest/download/v2ray-linux-64.zip
unzip v2ray-linux-64.zip v2ray v2ctl
mv v2ray tools/clients/
mv v2ctl tools/clients/

gcc -Wall -fexceptions  -DCURL_STATICLIB -g -std=c++11   -c geoip.cpp -o obj/geoip.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -g -std=c++11   -c logger.cpp -o obj/logger.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -g -std=c++11   -c main.cpp -o obj/main.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -g -std=c++11   -c misc.cpp -o obj/misc.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -g -std=c++11   -c multithread_test.cpp -o obj/multithread_test.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -g -std=c++11   -c printout.cpp -o obj/printout.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -g -std=c++11   -c processes.cpp -o obj/processes.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -g -std=c++11   -c rapidjson_extra.cpp -o obj/rapidjson_extra.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -g -I/usr/local/include/freetype2 -I/usr/include/freetype2 -std=c++11   -c renderer.cpp -o obj/renderer.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -g -std=c++11   -c rulematch.cpp -o obj/rulematch.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -g -std=c++11   -c socket.cpp -o obj/socket.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -g -std=c++11   -c speedtestutil.cpp -o obj/speedtestutil.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -g -std=c++11   -c tcping.cpp -o obj/tcping.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -g -std=c++11   -c webget.cpp -o obj/webget.o
g++ -g -o stairspeedtest obj/geoip.o obj/logger.o obj/main.o obj/misc.o obj/multithread_test.o obj/printout.o obj/processes.o obj/rapidjson_extra.o obj/renderer.o obj/rulematch.o obj/socket.o obj/speedtestutil.o obj/tcping.o obj/webget.o -lcurl -lPNGwriter -lpng16 -lfreetype -lz -lssl -lcrypto -ldl -lpthread -lyaml-cpp -lbz2 -s -static

chmod +rx stairspeedtest pref.ini webgui.sh tools/clients/* tools/gui/* tools/misc/*
tar czf stairspeedtest_reborn_linux64.tar.gz stairspeedtest pref.ini webgui.sh tools/

