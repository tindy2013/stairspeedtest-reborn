#!/bin/bash
export MACOSX_DEPLOYMENT_TARGET=10.13
set -xe

brew reinstall make cmake automake autoconf libtool
brew reinstall libpng freetype rapidjson pcre2 libevent zlib bzip2 pkgconfig

#git clone https://github.com/curl/curl --depth=1
#cd curl
#./buildconf > /dev/null
#./configure --with-ssl=/usr/local/opt/openssl@1.1 --without-mbedtls --disable-ldap --disable-ldaps --disable-rtsp --without-libidn2 > /dev/null
#cmake -DHTTP_ONLY=ON -DBUILD_TESTING=OFF -DBUILD_SHARED_LIBS=OFF -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl@1.1 -DCMAKE_USE_LIBSSH2=OFF . > /dev/null
#make -j8 > /dev/null
#cd ..

git clone https://github.com/jbeder/yaml-cpp --depth=1
cd yaml-cpp
cmake -DYAML_CPP_BUILD_TESTS=OFF -DYAML_CPP_BUILD_TOOLS=OFF . > /dev/null
make install -j8 > /dev/null
cd ..

git clone https://github.com/pngwriter/pngwriter --depth=1
cd pngwriter > /dev/null
cmake -DCMAKE_FIND_FRAMEWORK=NEVER . > /dev/null
sudo make install -j8 > /dev/null
cd ..

cp /usr/local/lib/libevent.a .
cp /usr/local/opt/zlib/lib/libz.a .
cp /usr/local/opt/openssl@1.1/lib/libssl.a .
cp /usr/local/opt/openssl@1.1/lib/libcrypto.a .
cp /usr/local/lib/libpcre2-8.a .
cp /usr/local/opt/bzip2/lib/libbz2.a .
cp /usr/local/lib/libPNGwriter.a .
cp /usr/local/lib/libpng.a .
cp /usr/local/lib/libfreetype.a .

export CMAKE_CXX_FLAGS="-I/usr/local/include -I/usr/local/opt/openssl@1.1/include -I/usr/local/opt/curl/include"
cmake -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl@1.1 -DOPENSSL_USE_STATIC_LIBS=TRUE -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13 .
make -j8
rm stairspeedtest
c++ -Xlinker -unexported_symbol -Xlinker "*" -o base/stairspeedtest -framework CoreFoundation -framework Security $(find CMakeFiles/stairspeedtest.dir/src/ -name "*.o") $(find . -name "*.a") -lcurl -ldl -lpthread -O3

set +xe
