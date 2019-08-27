#!/bin/bash
mkdir obj
mkdir bin
set -e
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -g -std=c++11   -c geoip.cpp -o obj/geoip.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -g -std=c++11   -c logger.cpp -o obj/logger.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -g -std=c++11   -c main.cpp -o obj/main.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -g -std=c++11   -c misc.cpp -o obj/misc.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -g -std=c++11   -c multithread-test.cpp -o obj/multithread-test.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -g -std=c++11   -c printout.cpp -o obj/printout.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -g -std=c++11   -c processes.cpp -o obj/processes.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -I/usr/local/include/freetype2 -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -g -std=c++11   -c renderer.cpp -o obj/renderer.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -g -std=c++11   -c rulematch.cpp -o obj/rulematch.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -g -std=c++11   -c socket.cpp -o obj/socket.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -g -std=c++11   -c speedtestutil.cpp -o obj/speedtestutil.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -g -std=c++11   -c tcping.cpp -o obj/tcping.o
c++ -I/usr/local/include -I/usr/local/opt/openssl@1.1/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -g -std=c++11   -c webget.cpp -o obj/webget.o
c++ -g -o stairspeedtest obj/geoip.o obj/logger.o obj/main.o obj/misc.o obj/multithread-test.o obj/printout.o obj/processes.o obj/renderer.o obj/rulematch.o obj/socket.o obj/speedtestutil.o obj/tcping.o obj/webget.o -L/usr/local/opt/openssl@1.1/lib -lcurl -lPNGwriter -lpng -lfreetype -lz -lssl -lcrypto -lyaml-cpp -ldl -lpthread
chmod +x stairspeedtest

