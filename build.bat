gcc -Wall -fexceptions -D_WIN32 -DCURL_STATICLIB -g -std=c++11  -c geoip.cpp -o obj\geoip.o
gcc -Wall -fexceptions -D_WIN32 -DCURL_STATICLIB -g -std=c++11  -c logger.cpp -o obj\logger.o
gcc -Wall -fexceptions -D_WIN32 -DCURL_STATICLIB -g -std=c++11  -c main.cpp -o obj\main.o
gcc -Wall -fexceptions -D_WIN32 -DCURL_STATICLIB -g -std=c++11  -c misc.cpp -o obj\misc.o
gcc -Wall -fexceptions -D_WIN32 -DCURL_STATICLIB -g -std=c++11  -c multithread-test.cpp -o obj\multithread-test.o
gcc -Wall -fexceptions -D_WIN32 -DCURL_STATICLIB -g -std=c++11  -c printout.cpp -o obj\printout.o
gcc -Wall -fexceptions -D_WIN32 -DCURL_STATICLIB -g -std=c++11  -c processes.cpp -o obj\processes.o
gcc -Wall -fexceptions -D_WIN32 -DCURL_STATICLIB -g -std=c++11  -c renderer.cpp -o obj\renderer.o
gcc -Wall -fexceptions -D_WIN32 -DCURL_STATICLIB -g -std=c++11  -c rulematch.cpp -o obj\rulematcho
gcc -Wall -fexceptions -D_WIN32 -DCURL_STATICLIB -g -std=c++11  -c socket.cpp -o obj\socket.o
gcc -Wall -fexceptions -D_WIN32 -DCURL_STATICLIB -g -std=c++11  -c speedtestutil.cpp -o obj\speedtestutil.o
gcc -Wall -fexceptions -D_WIN32 -DCURL_STATICLIB -g -std=c++11  -c tcping.cpp -o obj\tcping.o
gcc -Wall -fexceptions -D_WIN32 -DCURL_STATICLIB -g -std=c++11  -c webget.cpp -o obj\webget.o
windres   -J rc -O coff -i res\res.rc -o obj\res\res.res
g++  -o stairspeedtest.exe obj/geoip.o obj/logger.o obj/main.o obj/misc.o obj/multithread-test.o obj/printout.o obj/processes.o obj/renderer.o obj/rulematch.o obj/socket.o obj/speedtestutil.o obj/tcping.o obj/webget.o obj\res\res.res -lpsapi -lyaml-cpp -lcurl -lPNGwriter -lpng16 -lfreetype -lz -lssl -lcrypto -lws2_32 -lwsock32 
