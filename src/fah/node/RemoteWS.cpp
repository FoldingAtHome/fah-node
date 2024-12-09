/******************************************************************************\

                   This file is part of FoldingAtHome/fah-node

        The fah-node enables secure remote access to Folding@home clients.
                      Copyright (c) 2023, foldingathome.org
                               All rights reserved.

       This program is free software; you can redistribute it and/or modify
       it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 3 of the License, or
                       (at your option) any later version.

         This program is distributed in the hope that it will be useful,
          but WITHOUT ANY WARRANTY; without even the implied warranty of
          MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
                   GNU General Public License for more details.

     You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
           51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

                  For information regarding this software email:
                                 Joseph Coffland
                          joseph@cauldrondevelopment.com

\******************************************************************************/

#include "RemoteWS.h"
#include "App.h"
#include "Server.h"
#include "Account.h"

#include <cbang/openssl/Digest.h>
#include <cbang/log/Logger.h>
#include <cbang/http/Conn.h>

using namespace std;
using namespace cb;
using namespace FAH::Node;


RemoteWS::RemoteWS(
  App &app, const SmartPointer<HTTP::Conn> &connection,
  const URI &uri, const Version &version) :
  cb::WS::JSONWebsocket(connection, uri, version), app(app) {}


RemoteWS::~RemoteWS() {}


void RemoteWS::onMessage(const JSON::ValuePtr &msg) {
  string type = msg->getString("type", "");
  onMessage(type, msg);
  app.getStats().event(getWSType() + "-msg-" + type);
}


void RemoteWS::onComplete() {
  app.getStats().event(getWSType() + "-disconnect");
  app.getServer().remove(*this);
}


void RemoteWS::onLogin(const JSON::ValuePtr &msg) {
  auto payload     = msg->get("payload");
  string signature = msg->getString("signature");
  string pubkey    = msg->getString("pubkey");

  // Verify signature
  KeyPair key;
  key.readPublicSPKI(Base64().decode(pubkey));
  key.verifyBase64SHA256(signature, payload->toString(0, true));

  // Check timestamp is within allowed range
  uint64_t now = Time::now();
  uint64_t ts  = Time::parse(payload->getString("time"));
  const unsigned grace = 300;
  if (ts < now - grace || now + grace < ts)
    THROW("Login timestamp invalid: " << payload->getString("time"));

  // Compute ID from public key
  id = Digest::urlBase64(key.getRSA_N().toBinString(), "sha256");

  login = msg;

  // Clear Connection TTL
  getConnection()->setTTL(0);
}
