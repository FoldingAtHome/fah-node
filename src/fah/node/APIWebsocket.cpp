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

#include <cbang/http/Conn.h>

using namespace std;
using namespace cb;
using namespace FAH::Node;


APIWebsocket::APIWebsocket(App &app) : app(app) {}


void APIWebsocket::onMessage(const JSON::ValuePtr &msg) {
  sendMessage(msg->getString("request"), msg->getU64("id"));
}


void APIWebsocket::onOpen() {getConnection()->setTTL(0);} // Live forever
void APIWebsocket::onShutdown() {app.getServer().remove(*this);}


void APIWebsocket::sendMessage(const string &type, uint64_t id) {
  send([&] (JSON::Sink &sink) {
    sink.beginDict();
    sink.insert("id", id);
    sink.beginInsert("data");

    auto &server = app.getServer();
    if      (type == "server")      server.writeServer(sink);
    else if (type == "info")        server.writeInfo(sink);
    else if (type == "stats")       server.writeStats(sink);
    else if (type == "connections") server.writeConnections(sink);
    else if (type == "help")        server.writeHelp(sink);
    else THROW("Unknown message type '" << type << "'");

    sink.endDict();
  });
}
