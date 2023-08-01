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

#include "ClientWS.h"
#include "App.h"
#include "Account.h"

#include <cbang/Catch.h>
#include <cbang/openssl/Digest.h>

using namespace std;
using namespace cb;
using namespace FAH::Node;


ClientWS::~ClientWS() {}


void ClientWS::connect(const JSON::ValuePtr &msg) {
  send(*msg);
}


void ClientWS::disconnect(uint64_t ch) {
  JSON::ValuePtr msg = new JSON::Dict;
  msg->insert("type", "disconnect");
  msg->insert("ch",   ch);
  send(*msg);
}


void ClientWS::onMessage(const JSON::ValuePtr &msg) {
  try {
    string type = msg->getString("type", "");

    if (type == "register") {
      account = app.getAccount(msg->getString("account"));
      pubKey  = new KeyPair(msg->getString("pub"));
      id      = Digest::base64(pubKey->getPublic().toBinString(), "sha256");
      account->add(this);

      return;
    }

    if (account.isNull()) THROW("Client not registered");

    if (type == "message") {
      // TODO Forward "message" to account channel
      return;
    }

    THROW("Invalid message type '" << type << "'");
  } CATCH_ERROR;

  close(Event::WebsockStatus::WS_STATUS_UNACCEPTABLE);
}


void ClientWS::onOpen() {
  // TODO Timeout if client does not "register"
}


void ClientWS::onComplete() {
  if (account.isSet()) account->remove(*this);
  RemoteWS::onComplete();
}
