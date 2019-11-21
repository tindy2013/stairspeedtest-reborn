#!/bin/bash
mkdir obj
mkdir bin
set -e
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++11   -c geoip.cpp -o obj/geoip.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++11   -c logger.cpp -o obj/logger.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++11   -c main.cpp -o obj/main.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++11   -c misc.cpp -o obj/misc.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++11   -c multithread_test.cpp -o obj/multithread_test.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++11   -c printout.cpp -o obj/printout.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++11   -c processes.cpp -o obj/processes.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++11   -c rapidjson_extra.cpp -o obj/rapidjson_extra.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -I/usr/local/include/freetype2 -I/usr/include/freetype2 -std=c++11   -c renderer.cpp -o obj/renderer.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++11   -c rulematch.cpp -o obj/rulematch.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++11   -c socket.cpp -o obj/socket.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++11   -c speedtestutil.cpp -o obj/speedtestutil.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++11   -c tcping.cpp -o obj/tcping.o
gcc -Wall -fexceptions  -DCURL_STATICLIB -std=c++11   -c webget.cpp -o obj/webget.o
g++ -o stairspeedtest obj/geoip.o obj/logger.o obj/main.o obj/misc.o obj/multithread_test.o obj/printout.o obj/processes.o obj/rapidjson_extra.o obj/renderer.o obj/rulematch.o obj/socket.o obj/speedtestutil.o obj/tcping.o obj/webget.o -levent -lcurl -lPNGwriter -lpng16 -lfreetype -lz -lssl -lcrypto -ldl -lpthread -lyaml-cpp -s -static
chmod +x stairspeedtest

