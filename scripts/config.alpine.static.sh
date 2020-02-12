#!/bin/bash
set -e

apk add gcc g++ cmake make autoconf automake libtool

apk add libpng-dev libressl-dev freetype-dev freetype-static zlib-dev openssl rapidjson-dev pcre2-dev

git clone https://github.com/curl/curl
cd curl
./buildconf
./configure
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
