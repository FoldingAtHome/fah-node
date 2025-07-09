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

#include "AccountWS.h"
#include "App.h"
#include "Account.h"
#include "ClientWS.h"

#include <cbang/log/Logger.h>

using namespace std;
using namespace cb;
using namespace FAH::Node;


AccountWS::~AccountWS() {}


void AccountWS::connected(const ClientWS &client) {
  JSON::ValuePtr msg = new JSON::Dict;
  msg->insert("type", "connect");
  msg->insert("client", client.getLogin());
  send(*msg);
}


void AccountWS::disconnected(const string &id) {
  JSON::ValuePtr msg = new JSON::Dict;
  msg->insert("type", "disconnect");
  msg->insert("id",   id);
  send(*msg);
}


void AccountWS::onMessage(const string &type, const JSON::ValuePtr &msg) {
  if (type == "login") {
    onLogin(msg);

    sid = msg->selectString("payload.session");
    app.getAccount(getID())->add(this);

    LOG_INFO(3, "Account " << getID() << " logged");
    return;
  }

  if (account.isNull()) THROW("Account not logged in");

  // Forward messages to client
  if (type == "message") {
    const string &cid = msg->getString("id");
    auto client = account->findClient(cid);
    LOG_DEBUG(3, "Account " << getID() << " sending message to " << cid);

    if (client.isSet()) return client->send(*msg);
    else {
      LOG_WARNING("Client " << cid << " not found");
      return;
    }
  }

  // Broadcast account state to all clients and accounts
  if (type == "broadcast") return account->broadcast(msg);

  THROW("Invalid message type '" << type << "'");
}


void AccountWS::onOpen() {
  // TODO timeout connection if it does not login
}


void AccountWS::onClose(WS::Status status, const string &msg) {
 if (account.isSet()) account->removeAccount(sid);
 RemoteWS::onClose(status, msg);
}
