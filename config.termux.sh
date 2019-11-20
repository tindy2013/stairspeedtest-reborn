#!/bin/bash
set -e
pkg upgrade

#first install core utilities
pkg install -y git cmake clang autoconf automake libtool

#then install precompiled library from repo
pkg install -y openssl-static libcurl-static libpng-static libev-static libz-static freetype-static libevent

#get rapidjson
git clone https://github.com/tencent/rapidjson
cd rapidjson
cp -r include/* /data/data/com.termux/files/usr/include/
cd ..

#get pngwriter
git clone https://github.com/pngwriter/pngwriter
cd pngwriter
cmake -D CMAKE_INSTALL_PREFIX=/data/data/com.termux/files/usr .
make install -j2
cd ..

#get yaml-cpp
git clone https://github.com/jbeder/yaml-cpp
cd yaml-cpp
cmake -D CMAKE_INSTALL_PREFIX=/data/data/com.termux/files/usr .
make install -j2
cd ..

echo all done!
