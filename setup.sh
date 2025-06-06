#!/bin/bash

set -e

mkdir -p third_party
cd third_party

echo "Downloading Boost headers..."
BOOST_VERSION=1.88.0
BOOST_VERSION_UNDERSCORE=${BOOST_VERSION//./_}
wget -nc https://archives.boost.io/release/${BOOST_VERSION}/source/boost_${BOOST_VERSION_UNDERSCORE}.tar.gz
tar -xf boost_${BOOST_VERSION_UNDERSCORE}.tar.gz

echo "Copying boost headers..."
cp -r boost_${BOOST_VERSION_UNDERSCORE}/boost ../boost

echo "Downloading nlohmann json..."
wget -nc https://github.com/nlohmann/json/releases/latest/download/json.hpp -O json.hpp
mkdir -p ../nlohmann
mv json.hpp ../nlohmann/json.hpp

echo "Downloading SQLiteCpp..."
git clone --depth=1 https://github.com/SRombauts/SQLiteCpp.git
cp -r SQLiteCpp/include/SQLiteCpp ../SQLiteCpp
cp -r SQLiteCpp/sqlite3 ../SQLiteCpp/sqlite3

echo "Cleaning up..."
cd ..
rm -rf third_party

echo "✅ All dependencies are set up."