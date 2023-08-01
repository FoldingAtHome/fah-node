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

#include <cbang/Catch.h>

using namespace std;
using namespace cb;
using namespace FAH::Node;


AccountWS::~AccountWS() {}


void AccountWS::notify(const ClientWS &client) {
  JSON::ValuePtr msg = new JSON::Dict;
  msg->insert("type", "client");
  msg->insert("pub", client.getPubKey()->publicToString());
  send(*msg);
}


void AccountWS::onMessage(const JSON::ValuePtr &msg) {
  try {
    string type = msg->getString("type", "");

    if (type == "login") {
      SmartPointer<Certificate> cert = new Certificate(msg->getString("cert"));

      // TODO Check cert is valid

      account    = app.getAccount(cert->getNameEntry("CN"));
      this->cert = cert;
      chain      = cert->getPublicKey()->publicToString();
      // TODO Add API certificate to chain
      account->add(this);

      return;
    }

    if (account.isNull()) THROW("Account not logged in");

    // Forward messages to client
    if (type == "connect" || type == "message") {
      const auto &client = account->getClient(msg->getString("id"));
      msg->insert("id", getConnID()); // Change ID
      if (type == "connect") msg->insert("chain", chain);
      client->send(*msg);
      return;
    }

    THROW("Invalid message type '" << type << "'");
  } CATCH_ERROR;

  close(Event::WebsockStatus::WS_STATUS_UNACCEPTABLE);
}


void AccountWS::onOpen() {
  // TODO timeout connection if it does not login
}


void AccountWS::onComplete() {
  if (account.isSet()) account->remove(*this);
  RemoteWS::onComplete();
}
