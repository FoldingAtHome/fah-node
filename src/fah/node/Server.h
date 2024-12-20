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

#include <cstdint>
#include <string>
#include <map>


namespace FAH {
  namespace Node {
    class App;
    class RemoteWS;

    class Server : public cb::HTTP::Server {
    protected:
      App &app;
      cb::OAuth2::GoogleProvider googleOAuth2;
      cb::ACLSet aclSet;

      typedef cb::SmartPointer<cb::HTTP::Request> RequestPtr;
      std::map<uint64_t, RequestPtr> websockets;

    public:
      Server(App &app);
      ~Server();

      const RequestPtr &add(const RequestPtr &ws);
      void remove(const cb::HTTP::Request &ws);

      void initSSL(cb::SSLContext &ctx);
      void initHandlers();
      void init();

      // From cb::HTTP::Server
      using cb::HTTP::Server::init;
      cb::SmartPointer<cb::HTTP::Request> createRequest
      (const cb::SmartPointer<cb::HTTP::Conn> &connection,
       cb::HTTP::Method method, const cb::URI &uri,
       const cb::Version &version) override;

      void writeServer     (cb::JSON::Sink &sink) const;
      void writeInfo       (cb::JSON::Sink &sink) const;
      void writeRates      (cb::JSON::Sink &sink) const;
      void writeStats      (cb::JSON::Sink &sink) const;
      void writeConnections(cb::JSON::Sink &sink) const;
      void writeHelp       (cb::JSON::Sink &sink) const;

    protected:
      typedef cb::HTTP::Request EReq;
      typedef cb::JSON::ValuePtr JVP;

      bool apiCORS         (EReq &req);
      bool apiNotFound     (EReq &req);
      bool apiXFrameOptions(EReq &req);
      void apiLogout       (EReq &req, const JVP &msg);
      void apiServer       (EReq &req, const JVP &msg);
      void apiInfo         (EReq &req, const JVP &msg);
      void apiStats        (EReq &req, const JVP &msg);
      void apiConnections  (EReq &req, const JVP &msg);
      void apiHelp         (EReq &req, const JVP &msg);

      // Webpage handler methods
      bool forceSecure     (EReq &req);
      bool loginPage       (EReq &req);
    };
  }
}
