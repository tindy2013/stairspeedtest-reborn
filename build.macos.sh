#!/bin/bash
mkdir obj
mkdir bin
set -xe
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c geoip.cpp -o obj/geoip.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c logger.cpp -o obj/logger.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c main.cpp -o obj/main.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c misc.cpp -o obj/misc.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c multithread_test.cpp -o obj/multithread_test.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c printout.cpp -o obj/printout.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c processes.cpp -o obj/processes.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c rapidjson_extra.cpp -o obj/rapidjson_extra.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -I/usr/local/include/freetype2 -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c renderer.cpp -o obj/renderer.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c rulematch.cpp -o obj/rulematch.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c socket.cpp -o obj/socket.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c speedtestutil.cpp -o obj/speedtestutil.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c tcping.cpp -o obj/tcping.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c webget.cpp -o obj/webget.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c webgui_wrapper.cpp -o obj/webgui_wrapper.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -std=c++17   -c webserver_libevent.cpp -o obj/webserver_libevent.o
c++ -o stairspeedtest obj/geoip.o obj/logger.o obj/main.o obj/misc.o obj/multithread_test.o obj/printout.o obj/processes.o obj/rapidjson_extra.o obj/renderer.o obj/rulematch.o obj/socket.o obj/speedtestutil.o obj/tcping.o obj/webget.o obj/webgui_wrapper.o obj/webserver_libevent.o -L/usr/local/opt/openssl@1.1/lib -lcurl -lPNGwriter -lpng -lfreetype -lz -lssl -lcrypto -lyaml-cpp -ldl -lpthread
chmod +x stairspeedtest
