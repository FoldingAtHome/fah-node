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
#include "AccountWS.h"

#include <cbang/Catch.h>

using namespace std;
using namespace cb;
using namespace FAH::Node;


ClientWS::~ClientWS() {}


void ClientWS::closeSession(const string &sid) {
  JSON::Dict msg;
  msg.insert("type", "session-close");
  msg.insert("session", sid);
  send(msg);
}


void ClientWS::onMessage(const JSON::ValuePtr &msg) {
  string type = msg->getString("type");

  if (type == "login") {
    onLogin(msg);

    // Add to account
    string aid = msg->selectString("payload.account");
    account = app.getAccount(aid);
    account->add(this);

    LOG_INFO(3, "Client " << getID() << " logged in to " << aid);
    return;
  }

  if (account.isNull()) THROW("Client not logged in");

  if (type == "message") {
    // Forward "message" to account session
    string sid = msg->getString("session");
    auto session = account->getSession(sid);

    if (session.isSet()) {
      session->send(*msg);
      LOG_DEBUG(3, "Client " << getID() << " sent message to account "
                << account->getID() << " session " << sid);

    } else closeSession(sid);

  } else if (type == "status") {
    // TODO handle client status messages

  } else THROW("Invalid message type '" << type << "'");
}


void ClientWS::onOpen() {
  // TODO Timeout if client does not "login"
}


void ClientWS::onComplete() {
  if (account.isSet()) account->remove(*this);
  RemoteWS::onComplete();
}
