#!/bin/bash

apk add gcc g++ build-base linux-headers cmake make autoconf automake libtool git
apk add libpng-dev libpng-static libressl-dev freetype-dev freetype-static zlib-dev zlib-static rapidjson-dev libevent-dev libevent-static bzip2-static

git clone https://github.com/curl/curl
cd curl
./buildconf > /dev/null
./configure --with-ssl --disable-ldap --disable-ldaps --disable-rtsp --without-libidn2 > /dev/null
make install -j4 > /dev/null
cd ..

git clone https://github.com/jbeder/yaml-cpp
cd yaml-cpp
cmake -DYAML_CPP_BUILD_TESTS=OFF -DYAML_CPP_BUILD_TOOLS=OFF . > /dev/null
make install -j4 > /dev/null
cd ..

git clone https://github.com/pngwriter/pngwriter
cd pngwriter
cmake . > /dev/null
make install -j4 > /dev/null
cd ..

cmake .
make -j4
g++ -o base/stairspeedtest CMakeFiles/stairspeedtest.dir/src/*.o  -static -lpcrecpp -lpcre -levent -lyaml-cpp -lPNGwriter -lpng -lfreetype -lcurl -lssl -lcrypto -lz -lbz2 -ldl -lpthread -O3 -s  

if [ $TRAVIS_BRANCH = $TRAVIS_TAG ];then
	bash scripts/build.alpine.clients.sh
	
	cd base
	chmod +rx stairspeedtest *.sh
	chmod +r *

	tar czf ../stairspeedtest_reborn_linux64.tar.gz *
fi
