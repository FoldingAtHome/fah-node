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

#pragma once

#include <cbang/http/Server.h>
#include <cbang/util/ACLSet.h>
#include <cbang/json/JSON.h>
#include <cbang/oauth2/GoogleProvider.h>
#include <cbang/ws/Websocket.h>

#include <cstdint>
#include <string>
#include <map>


namespace FAH {
  namespace Node {
    class App;
    class RemoteWS;
    class APIWebsocket;

    class Server : public cb::HTTP::Server {
    protected:
      App &app;
      cb::OAuth2::GoogleProvider googleOAuth2;
      cb::ACLSet aclSet;

      using WebsocketPtr = cb::WS::WebsocketPtr;
      std::map<uint64_t, WebsocketPtr> websockets;

    public:
      Server(App &app);
      ~Server();

      const WebsocketPtr &add(const WebsocketPtr &ws);
      void remove(const cb::WS::Websocket &ws);

      void initSSL(cb::SSLContext &ctx);
      void initHandlers();
      void init();

      void writeServer     (cb::JSON::Sink &sink) const;
      void writeInfo       (cb::JSON::Sink &sink) const;
      void writeStats      (cb::JSON::Sink &sink) const;
      void writeConnections(cb::JSON::Sink &sink) const;
      void writeHelp       (cb::JSON::Sink &sink) const;

      using cb::HTTP::Server::init;

    protected:
      using Request  = cb::HTTP::Request;
      using ValuePtr = cb::JSON::ValuePtr;

      bool apiCORS         (Request &req);
      bool websocket       (Request &req);
      bool apiNotFound     (Request &req);
      bool apiXFrameOptions(Request &req);
      void apiLogout       (Request &req, const ValuePtr &msg);
      void apiServer       (Request &req, const ValuePtr &msg);
      void apiInfo         (Request &req, const ValuePtr &msg);
      void apiStats        (Request &req, const ValuePtr &msg);
      void apiConnections  (Request &req, const ValuePtr &msg);
      void apiHelp         (Request &req, const ValuePtr &msg);

      // Webpage handler methods
      bool forceSecure     (Request &req);
      bool loginPage       (Request &req);
    };
  }
}
