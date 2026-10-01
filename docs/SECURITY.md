fah-node Security Protocol
==========================

This document describes the protocol used to secure communication in a
Folding@home swarm using ``fah-node``.

## Account Registration

User accounts are authenticated using standard security protocols including
X.509, RSA-OAEP and PBKDF2.  A F@H account is registered by creating a new
RSA key pair, encrypting the private key with the password and storing the
encrypted result on the api.foldingathome.org server.  The account is activated
once the user prooves they have control over the supplied email address by
responding with emailed random token.

Pseudo code for account creation proceeds as follows:

  1. User provides passphrase & email
  1. K = RSA-OAEP.new()
  1. P = K.public
  1. S = SHA256(email.toLowerCase())
  1. L = PBKDF2.derive(passphrase, S)
  1. W = L.wrap(K.private, S[0:16])
  1. H = SHA256(L)
  1. H, W & P are sent to api.foldingathome.org
  1. The API stores ``password = SHA256(H)``, ``secret = W``, ``pubkey = P`` in
     the database.

Note that the ``password`` stored in the database is not the same as the
``passphrase`` supplied by the user.  The ``passphrase`` never leaves the
browser, only the hash of the PBKDF2 derived passphrase is sent to the API and
this is impossible to reverse.  The API then hashes this again before stroing
it in the DB along with the encrypted private key.

## Account Login

To login the browser needs the account's private key.  The following procecure
is used to recover the private key:

  1. Derive the passphrase hash to proove ownership of the account:
    1. User provides passphrase & email
    1. S = SHA256(email.toLowerCase())
    1. L = PBKDF2.derive(passphrase, S)
    1. H = SHA256(L)
  1. Request the encrypted private key by sending ``email`` and ``H``
     api.foldingathome.org.
  1. The API computes ``password = SHA256(H)`` and returns ``secret`` if it
     matches.
  1. The browser unlocks the private key as follows:
    1. K = W.unwrap(L, S[0:16])
  1. Finally, the account ID is computed as a SH256 hash of the account public
     key.

## Account to Node Authentication

The client web interface connects to the node via a secure Websocket on the
``/account`` endpoint and submits a signed login message to the node for
authentication.

```javascript
{
  "type": "login",
  "payload": {
    "time": "<current ISO8601 date and time>",
    "session": "<base64 encoded random 16 bytes>"
  },
  "pubkey": "<account public key>",
  "signature": "<payload signature>"
}
```

The node performs the following operations:

 1. The signature is verified against the provided public key.
 1. The timestamp is checked to ensure it was created within the last 5 minutes.
 1. The account ID is computed from the public key.
 1. If the checks pass the login is approved, otherwise disconnected.

Once the account is authenticated it may hold the websocket open indefinately
and send and receive messages to/from clients which are configured for the
account.

## Client Registration

When a folding client first starts it generates it's own public/private key pair
which it stores in it's local DB.  This key pair is stored unencrypted.  Its
secuirty relies on the machine it runs on being secure.  If the private key were
to be leaked it would allow another machine to impersonate the orignal machine
but would *not* give an attacker remote access to the original machine.

Folding client's must opt-in to a Folding@home account.  To do so they require
the account's current ``token``.  The account token is a random string of
32 bytes that may be changed at anytime by the account holder.  Given the
account token a client will send a message to api.foldingathome.org requesting
to join the account.

```javascript
{
  "data": {
    "name": "<machine display name>",
    "token": "<current account token>"
  },
  "pubkey": "<account public key>",
  "signature": "<data signature>"
}
```

The F@H API then verifies the signature and checks the token.  If valid the
machine's public key and name are added to the account.

## Client to Node Authentication

Clients login to the node by connecting to the secure Websocket at the
``/client`` endpoint and sending a login message:

```javascript
  "type": "login",
  "payload": {
    "time": "<current ISO8601 date and time>",
    "account": "<account ID>",
    "key": "<encrypted session key>"
  },
  "pubkey": "<account public key>",
  "signature": "<payload signature>"
```

The client computes a random 32-byte session key then encrypts it using the
account's public key.  This ensures that only the account can decrypt the
session key and use it to communicate with the client.

******************

## Node to account communication

The account opens a WWS connection to the node.  At which point the node may
send the following messages:

### Client

```
{
  "type":   "client",
  "pub":    <pub_key>,
}
```

This indicates that a client associated with the account is connected to the
node.

``<pub_key>`` in in PEM format.  The client ID is computed as the URL base 64
encoded SHA256 hash of the ``<pub_key>``.

### Message

```
{
  "type":     "message",
  "id":       <client_id>,
  "data":     <encrypted>
}
```

The data will be encrypted using the key sent by the account and URL base 64
encoded.

### Disconnect

```
{
  "type":   "disconnect",
  "id":     <client_id>
}
```

The client is no longer connected to the node.

## Account to node communication

The account may send the following messages to the node:

### Login

```
{
  "type": "login",
  "cert": <x509>
}
```

``<x509>`` is the account's certificate signed by the API. The node will verify
the certificate and disconnect the account if verification fails.  Only after
this message can the account send other messages.

### Connect

```
{
  "type": "connect",
  "id":   <client_id>,
  "key":  <key>,
  "sig":  <signature>
}
```

Where ``<key>`` is an encryption key encrypted with the client's public key.
``<signature>`` is a signature on ``<client_id>:<key>`` with secret key behind
the cert provided at login.

### Message
```
{
  "type": "message",
  "id":   <client_id>,
  "data": <encrypted>
}
```

## Node to client communication

The client may receive the following messages from the node:

### Connect

```
{
  "type":  "connect",
  "id":    <channel_id>,
  "key":   <key>,
  "sig":   <signature>,
  "chain": <x509_chain>
}
```

### Disconnect

```
{
  "type": "disconnect",
  "id":   <channel_id>
}
```

Indicates the account channel is no longer connected.

### Message
```
{
  "type": "message",
  "ch":   <u64>,
  "data": <encrypted>
}
```

## Client to node communication

Once a client is connected to the node it may send the following messages:

### Register

```
{
  "type":    "register",
  "pub":     <pub_key>,
  "account": <account_id>
}
```

### Message

```
{
  "type": "message",
  "ch":   <u64>,
  "data": <encrypted>
}
```

This message type may only be sent after a "connect" message is received.
