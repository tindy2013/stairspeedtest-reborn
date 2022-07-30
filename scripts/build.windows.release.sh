#!/bin/bash
set -xe

cmake -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" .
make -j4
rm stairspeedtest.exe

# this may change in the future
export FREETYPE_DEPS=$(pkg-config --libs --static harfbuzz)
# build resources
windres -J rc -O coff -i res/res.rc -o res.res
g++ $(find CMakeFiles/stairspeedtest.dir/src -name "*.obj") "$USERPROFILE/maindeps/curl/lib/libcurl.a" -lbcrypt res.res -o base/stairspeedtest.exe -static -levent -lPNGwriter -lfreetype $FREETYPE_DEPS -lpng -lpcre2-8 -lyaml-cpp -lssl -lcrypto -lws2_32 -lwsock32 -lcrypt32 -liphlpapi  -lz -lbz2 -s

set +xe
