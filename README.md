# fah-node
Authenticate, monitor and control your swarm of Folding@home clients remotely

## Debian Linux Quick Start

### Build
The following commands will build and install fah-node on Debian based Linux
systems.

```sh
sudo apt-get update
sudo apt-get install -y scons build-essential libssl-dev binutils-dev \
  libleveldb-dev libsnappy-dev git nodejs npm bind9-dnsutils ssl-cert

mkdir build
cd build

git clone https://github.com/cauldrondevelopmentllc/cbang
git clone https://github.com/foldingathome/fah-node

export CBANG_HOME=$PWD/cbang

scons -C cbang
scons -C fah-node
scons -C fah-node package
```

### Install

```sh
sudo dpkg -i fah-node/fah-node_$(cat fah-node/version.txt)_amd64.deb
sudo fah-node-ports
sudo apt-get install -y iptables-persistent
```

### Setup

TODO

## Architecture
