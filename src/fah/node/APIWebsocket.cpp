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

#include "APIWebsocket.h"

#include "App.h"
#include "Server.h"

#include <cbang/json/Builder.h>
#include <cbang/http/Conn.h>

using namespace std;
using namespace cb;
using namespace FAH::Node;


APIWebsocket::APIWebsocket(
  App &app, const SmartPointer<HTTP::Conn> &connection, const URI &uri,
  const Version &version) :
  cb::WS::JSONWebsocket(connection, uri, version), app(app) {}


void APIWebsocket::onMessage(const JSON::ValuePtr &msg) {
  JSON::Builder builder;
  builder.beginDict();
  builder.insert("id", *msg->get("id"));
  builder.beginInsert("data");

  auto &server = app.getServer();
  string req   = msg->getString("request");
  if (req == "server")      server.writeServer(builder);
  if (req == "info")        server.writeInfo(builder);
  if (req == "stats")       server.writeStats(builder);
  if (req == "connections") server.writeConnections(builder);
  if (req == "help")        server.writeHelp(builder);

  builder.endDict();

  send(*builder.getRoot());
}


void APIWebsocket::onOpen() {
  // Clear Connection TTL
  getConnection()->setTTL(0);
}


void APIWebsocket::onComplete() {app.getServer().remove(*this);}
