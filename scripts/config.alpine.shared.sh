#!/bin/bash
set -e

apk add gcc g++ cmake make

apk add libpng-dev curl-dev freetype-dev zlib-dev openssl rapidjson-dev yaml-cpp-dev pcre2-dev

git clone https://github.com/pngwriter/pngwriter
cd pngwriter
cmake .
make install -j4
cd ..
