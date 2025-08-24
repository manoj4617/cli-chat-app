#!/bin/bash

set -e

mkdir -p third_party
cd third_party

echo "Is Java 11 installed ?"
if ! java -version 2>&1 | awk -F '"' '/version/ {print $2}' | grep -E "^11"; then
    echo "❌ Java 11 is not installed or not set as the default. Please install Java 11 and try again."
    exit 1
fi
echo "✅ Java 11 is installed."

echo "Adding Cassandra repository..."
sudo mkdir -p /etc/apt/keyrings
curl https://downloads.apache.org/cassandra/KEYS | gpg --dearmor | sudo tee /etc/apt/keyrings/apache-cassandra.gpg > /dev/null
echo "deb [signed-by=/etc/apt/keyrings/apache-cassandra.gpg] https://debian.cassandra.apache.org 41x main" | sudo tee /etc/apt/sources.list.d/cassandra.list

echo "--- Step 1: Installing system packages (libsodium, Cassandra DB, and Cassandra C++ Driver) ---"
sudo apt-get update
sudo apt-get install -y \
    pkg-config \
    libsodium-dev \
    libuv1-dev \
    libssl-dev \
    zlib1g-dev\
    cassandra \

echo "System packages installed."
echo "--- Step 2: Checking Cassandra service status ---"
sudo service cassandra stop || true
sudo service cassandra start

echo "Waiting for Cassandra to initialize (this may take up to a minute)..."
sleep 45

echo "Verifying Cassandra node status..."
count=0
until nodetool status || [ $count -eq 5 ]; do
    echo "Cassandra not ready yet, retrying in 10 seconds..."
    sleep 10
    count=$((count+1))
done

if ! nodetool status; then
    echo "❌ Error: Cassandra service failed to start or is not responsive."
    exit 1
fi

echo "✅ Cassandra service is active and running."

echo ""
echo "--- Step 3: Installing Cassandra C++ Driver ---"
git clone https://github.com/datastax/cpp-driver.git
cd cpp-driver
mkdir build && cd build
cmake ..          # add -DCMAKE_BUILD_TYPE=Release if you want
make -j"$(nproc)"
sudo make install

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
if [ ! -d "SQLiteCpp" ]; then
    git clone --depth=1 https://github.com/SRombauts/SQLiteCpp.git
else
    echo "SQLiteCpp directory already exists, skipping clone."
fi
cp -r SQLiteCpp ../

echo "Cleaning up..."
cd ..
rm -rf third_party

echo "✅ All dependencies are set up."