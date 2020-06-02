#!/bin/bash
set -xe

apk add gcc g++ build-base linux-headers cmake make autoconf automake libtool git
apk add libpng-dev libpng-static openssl-dev openssl-libs-static curl-dev curl-static nghttp2-static freetype-dev freetype-static zlib-dev zlib-static rapidjson-dev libevent-dev libevent-static bzip2-static pcre2-dev brotli-static

git clone https://github.com/jbeder/yaml-cpp --depth=1
cd yaml-cpp
cmake -DYAML_CPP_BUILD_TESTS=OFF -DYAML_CPP_BUILD_TOOLS=OFF .
make install -j4
cd ..

git clone https://github.com/pngwriter/pngwriter --depth=1
cd pngwriter
cmake .
make install -j4
cd ..

cmake .
make -j4
rm stairspeedtest
g++ -o base/stairspeedtest CMakeFiles/stairspeedtest.dir/src/*.o  -static -lpcre2-8 -levent -lyaml-cpp -lPNGwriter -lpng -lfreetype -lcurl -lnghttp2 -lssl -lcrypto -lz -lbz2 -lbrotlidec-static -lbrotlicommon-static -ldl -lpthread -O3 -s  

if [ "$TRAVIS_BRANCH" = "$TRAVIS_TAG" ];then
	bash scripts/build.alpine.clients.sh
	
	cd base
	chmod +rx stairspeedtest *.sh
	chmod +r *
  cd ..
  mv base stairspeedtest
fi
