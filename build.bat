g++.exe -Wall -std=c++17 -D_WIN32 -DCURL_STATICLIB -O3 -c geoip.cpp -o obj\geoip.o
g++.exe -Wall -std=c++17 -D_WIN32 -DCURL_STATICLIB -O3 -c logger.cpp -o obj\logger.o
g++.exe -Wall -std=c++17 -D_WIN32 -DCURL_STATICLIB -O3 -c main.cpp -o obj\main.o
g++.exe -Wall -std=c++17 -D_WIN32 -DCURL_STATICLIB -O3 -c misc.cpp -o obj\misc.o
g++.exe -Wall -std=c++17 -D_WIN32 -DCURL_STATICLIB -O3 -c multithread_test.cpp -o obj\multithread_test.o
g++.exe -Wall -std=c++17 -D_WIN32 -DCURL_STATICLIB -O3 -c perf_test.cpp -o obj\perf_test.o
g++.exe -Wall -std=c++17 -D_WIN32 -DCURL_STATICLIB -O3 -c printout.cpp -o obj\printout.o
g++.exe -Wall -std=c++17 -D_WIN32 -DCURL_STATICLIB -O3 -c processes.cpp -o obj\processes.o
g++.exe -Wall -std=c++17 -D_WIN32 -DCURL_STATICLIB -O3 -c rapidjson_extra.cpp -o obj\rapidjson_extra.o
g++.exe -Wall -std=c++17 -D_WIN32 -DCURL_STATICLIB -O3 -c renderer.cpp -o obj\renderer.o
windres.exe   -J rc -O coff -i res\res.rc -o obj\res\res.res
g++.exe -Wall -std=c++17 -D_WIN32 -DCURL_STATICLIB -O3 -c rulematch.cpp -o obj\rulematch.o
g++.exe -Wall -std=c++17 -D_WIN32 -DCURL_STATICLIB -O3 -c socket.cpp -o obj\socket.o
g++.exe -Wall -std=c++17 -D_WIN32 -DCURL_STATICLIB -O3 -c speedtestutil.cpp -o obj\speedtestutil.o
g++.exe -Wall -std=c++17 -D_WIN32 -DCURL_STATICLIB -O3 -c tcping.cpp -o obj\tcping.o
g++.exe -Wall -std=c++17 -D_WIN32 -DCURL_STATICLIB -O3 -c webget.cpp -o obj\webget.o
g++.exe -Wall -std=c++17 -D_WIN32 -DCURL_STATICLIB -O3 -c webserver.cpp -o obj\webget.o
g++.exe -Wall -std=c++17 -D_WIN32 -DCURL_STATICLIB -O3 -c webgui_wrapper.cpp -o obj\webgui_wrapper.o
g++.exe -Wall -std=c++17 -D_WIN32 -DCURL_STATICLIB -O3 -c webserver_libevent.cpp -o obj\webserver_libevent.o
g++.exe -o bin\stairspeedtest.exe obj\geoip.o obj\logger.o obj\main.o obj\misc.o obj\multithread_test.o obj\perf_test.o obj\printout.o obj\processes.o obj\rapidjson_extra.o obj\renderer.o obj\rulematch.o obj\socket.o obj\speedtestutil.o obj\tcping.o obj\webget.o obj\webgui_wrapper.o obj\webserver_libevent.o obj\res\res.res -static -lyaml-cpp -lcurl -lPNGwriter -lpng16 -lfreetype -lz -lbz2 -lssl -lcrypto -lws2_32 -lwsock32 -s -static  
