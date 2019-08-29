apk add git gcc g++ cmake make autoconf

apk add libpng-dev curl-dev freetype-static zlib-dev openssl rapidjson-dev

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