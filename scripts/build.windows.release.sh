#!/bin/bash
set -xe

git clone https://github.com/curl/curl --depth=1
cd curl
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_USE_LIBSSH2=OFF -DHTTP_ONLY=ON -DCMAKE_USE_SCHANNEL=ON -DBUILD_SHARED_LIBS=OFF -DBUILD_CURL_EXE=OFF -DCMAKE_INSTALL_PREFIX=$MINGW_PREFIX -G "Unix Makefiles" .
make install -j4
cd ..

git clone https://github.com/jbeder/yaml-cpp --depth=1
cd yaml-cpp
cmake -DCMAKE_BUILD_TYPE=Release -DYAML_CPP_BUILD_TESTS=OFF -DYAML_CPP_BUILD_TOOLS=OFF -DCMAKE_INSTALL_PREFIX=$MINGW_PREFIX -G "Unix Makefiles" .
make install -j4
cd ..

git clone https://github.com/Tencent/rapidjson --depth=1
cd rapidjson
cmake -DRAPIDJSON_BUILD_DOC=OFF -DRAPIDJSON_BUILD_EXAMPLES=OFF -DRAPIDJSON_BUILD_TESTS=OFF -DCMAKE_INSTALL_PREFIX=$MINGW_PREFIX -G "Unix Makefiles" .
make install -j4
cd ..

git clone https://github.com/pngwriter/pngwriter --depth=1
cd pngwriter
cmake -DCMAKE_INSTALL_PREFIX=$MINGW_PREFIX -G "Unix Makefiles" .
make install -j4
cd ..

cmake -DCMAKE_BUILD_TYPE=Release -G "Unix Makefiles" .
make -j4
rm stairspeedtest.exe
# this may change in the future
export FREETYPE_DEPS="-lharfbuzz -lm -lusp10 -lgdi32 -lrpcrt4 -ldwrite -lfreetype -lbz2 -lpng16 -lz -lgraphite2 -lglib-2.0 -lintl -lws2_32 -lole32 -lwinmm -lshlwapi -pthread -lm -lpcre -lbrotlidec-static -lbrotlicommon-static"

g++ $(find CMakeFiles/stairspeedtest.dir/src -name "*.obj") curl/lib/libcurl.a -o base/stairspeedtest.exe -static -levent -lcurl -lPNGwriter -lfreetype $FREETYPE_DEPS -lpng -lpcre2-8 -lyaml-cpp -lssl -lcrypto -lws2_32 -lwsock32 -lcrypt32 -lz -lbz2 -s
