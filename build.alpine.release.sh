#!/bin/bash
mkdir obj
set -xe

apk add gcc g++ build-base linux-headers cmake make autoconf automake libtool
apk add libpng-dev libpng-static libressl-dev freetype-dev freetype-static zlib-dev rapidjson-dev mbedtls-dev mbedtls-static libevent-dev libevent-static

chmod +x build.alpine.clients.sh
./build.alpine.clients.sh

git clone https://github.com/curl/curl
cd curl
./buildconf
./configure > /dev/null
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

git clone git://sourceware.org/git/bzip2.git
cd bzip2
make install -j4
cd ..

gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++17   -c geoip.cpp -o obj/geoip.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++17   -c logger.cpp -o obj/logger.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++17   -c main.cpp -o obj/main.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++17   -c misc.cpp -o obj/misc.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++17   -c multithread_test.cpp -o obj/multithread_test.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++17   -c printout.cpp -o obj/printout.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++17   -c processes.cpp -o obj/processes.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++17   -c rapidjson_extra.cpp -o obj/rapidjson_extra.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -I/usr/local/include/freetype2 -I/usr/include/freetype2 -std=c++17   -c renderer.cpp -o obj/renderer.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++17   -c rulematch.cpp -o obj/rulematch.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++17   -c socket.cpp -o obj/socket.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++17   -c speedtestutil.cpp -o obj/speedtestutil.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++17   -c tcping.cpp -o obj/tcping.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++17   -c webget.cpp -o obj/webget.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++17   -c webgui_wrapper.cpp -o obj/webgui_wrapper.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++17   -c webserver_libevent.cpp -o obj/webserver_libevent.o
g++ -o stairspeedtest obj/geoip.o obj/logger.o obj/main.o obj/misc.o obj/multithread_test.o obj/printout.o obj/processes.o obj/rapidjson_extra.o obj/renderer.o obj/rulematch.o obj/socket.o obj/speedtestutil.o obj/tcping.o obj/webget.o obj/webgui_wrapper.o obj/webserver_libevent.o -levent -lcurl -lPNGwriter -lpng16 -lfreetype -lz -lssl -lcrypto -ldl -lpthread -lyaml-cpp -lbz2 -s -static

chmod +rx stairspeedtest pref.ini webgui.sh webserver.sh tools/clients/* tools/gui/* tools/misc/*
tar czf stairspeedtest_reborn_linux64.tar.gz stairspeedtest pref.ini webgui.sh webserver.sh tools/
