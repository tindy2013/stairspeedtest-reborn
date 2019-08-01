#!/bin/bash
mkdir obj
mkdir bin
set -e
gcc -I/usr/local/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -g -std=c++11   -c logger.cpp -o obj/logger.o
gcc -I/usr/local/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -g -std=c++11   -c main.cpp -o obj/main.o
gcc -I/usr/local/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -g -std=c++11   -c misc.cpp -o obj/misc.o
gcc -I/usr/local/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -g -std=c++11   -c multithread-test.cpp -o obj/multithread-test.o
gcc -I/usr/local/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -g -std=c++11   -c printout.cpp -o obj/printout.o
gcc -I/usr/local/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -g -std=c++11   -c processes.cpp -o obj/processes.o
gcc -I/usr/local/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -g -std=c++11   -c renderer.cpp -o obj/renderer.o
gcc -I/usr/local/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -g -std=c++11   -c socket.cpp -o obj/socket.o
gcc -I/usr/local/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -g -std=c++11   -c speedtestutil.cpp -o obj/speedtestutil.o
gcc -I/usr/local/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -g -std=c++11   -c tcping.cpp -o obj/tcping.o
gcc -I/usr/local/include -Wall -fexceptions -D_MACOS -DCURL_STATICLIB -g -std=c++11   -c webget.cpp -o obj/webget.o
g++ -g -o stairspeedtest obj/logger.o obj/main.o obj/misc.o obj/multithread-test.o obj/printout.o obj/processes.o obj/renderer.o obj/socket.o obj/speedtestutil.o obj/tcping.o obj/webget.o -L/usr/local/lib -lcurl -lPNGwriter -lpng16 -lfreetype -lz -lssl -lcrypto -lyaml-cpp -ldl -lpthread
chmod +x stairspeedtest

