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
#### Domain name
Add the node's domain name to ``config.xml``.

```xml
  <domains>node.example.com</domains>
```

#### Setup Google OAuth2
Follow these steps to configure OAuth2 logins for your node:

 # Go to https://console.developers.google.com/apis/dashboard
 # Login to your Google account
 # Create a new project
 # Configure the OAuth consent screen
 # Click on ``Credentials`` then ``CREATE CREDENTIALS``
 # Select ``OAuth client ID``
 # Select ``Application type``: ``Web application``
 # Give your OAuth application a name
 # Add the ``Authorized JavaScript origin``: ``https://node.example.com``
 # Add the ``Authorized redirect URI``: ``https://node.example.com/login``
 # Click the ``CREATE`` button
 # Note the ``Client ID`` and ``Client secret``.

Note, Google tends to change the interface fairly often so you may have to
adjust the above described process.

Add the following to your ``config.xml``:

```xml
  <google-client-id>YOUR_CLIENT_ID</google-client-id>
  <google-client-secret>YOUR_CLIENT_SECRET</google-client-secret>
  <google-redirect-base>https://node.example.com</google-redirect-base>
```

#### Increase system open file limit
As root, edit ``/etc/security/limits.conf``.  Add the following:

    # For F@H AS
    * soft nofile 10240
    * hard nofile 10240
    root soft nofile 10240
    root hard nofile 10240

Logout and back in for new limits to take effect.

## Architecture
