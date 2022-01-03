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
cmake -DYAML_CPP_BUILD_TESTS=OFF -DYAML_CPP_BUILD_TOOLS=OFF -DCMAKE_INSTALL_PREFIX=/opt/homebrew . > /dev/null
make install -j8 > /dev/null
cd ..

git clone https://github.com/pngwriter/pngwriter --depth=1
cd pngwriter > /dev/null
cmake -DCMAKE_FIND_FRAMEWORK=NEVER -DCMAKE_INSTALL_PREFIX=/opt/homebrew . > /dev/null
make install -j8 > /dev/null
cd ..

cp /opt/homebrew/opt/libevent/lib/libevent.a .
cp /opt/homebrew/opt/zlib/lib/libz.a .
cp /opt/homebrew/opt/openssl@1.1/lib/libssl.a .
cp /opt/homebrew/opt/openssl@1.1/lib/libcrypto.a .
cp /opt/homebrew/opt/pcre2/lib/libpcre2-8.a .
cp /opt/homebrew/opt/bzip2/lib/libbz2.a .
cp /opt/homebrew/opt/libpngwriter/lib/libPNGwriter.a .
cp /opt/homebrew/opt/libpng/lib/libpng.a .
cp /opt/homebrew/opt/freetype/lib/libfreetype.a .
cp yaml-cpp/libyaml-cpp.a .
cp pngwriter/libPNGwriter.a .

#export CMAKE_CXX_FLAGS="-I/opt/homebrew/opt/openssl@1.1/include -I/opt/homebrew/opt/curl/include"
cmake -DOPENSSL_ROOT_DIR=/opt/homebrew/opt/openssl@1.1 -DOPENSSL_USE_STATIC_LIBS=TRUE -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13 .
make -j8
rm stairspeedtest
c++ -Xlinker -unexported_symbol -Xlinker "*" -o base/stairspeedtest -framework CoreFoundation -framework Security $(find CMakeFiles/stairspeedtest.dir/src/ -name "*.o") ./*.a -lcurl -ldl -lpthread -O3

set +xe
