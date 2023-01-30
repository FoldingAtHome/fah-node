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

#include <cbang/StdTypes.h>

#include <cbang/net/IPAddress.h>
#include <cbang/event/WebServer.h>
#include <cbang/util/ACLSet.h>
#include <cbang/json/JSON.h>
#include <cbang/openssl/CertificateStore.h>
#include <cbang/auth/GoogleOAuth2.h>

#include <string>
#include <functional>


namespace cb {class Options;}

namespace FAH {
  class App;
  class Context;
  class Job;

  class Server : public cb::Event::WebServer,
                 public cb::Event::HTTPHandlerFactory {
  protected:
    App &app;
    cb::Options &options;
    cb::GoogleOAuth2 googleOAuth2;
    cb::ACLSet aclSet;

  public:
    Server(App &app);
    ~Server();

    void init(cb::SSLContext &ctx);
    void initHandlers();

    // From cb::Event::WebServer
    void init();

    // From cb::Event::HTTPHandler
    cb::SmartPointer<cb::Event::Request> createRequest
    (cb::Event::RequestMethod method, const cb::URI &uri,
     const cb::Version &version);

  protected:
    bool apiCORS(         cb::Event::Request &req);
    bool apiNotFound(     cb::Event::Request &req);
    bool apiXFrameOptions(cb::Event::Request &req);
    void apiLogout(     cb::Event::Request &req, const cb::JSON::ValuePtr &msg);
    void apiServer(     cb::Event::Request &req, const cb::JSON::ValuePtr &msg);
    void apiInfo(       cb::Event::Request &req, const cb::JSON::ValuePtr &msg);
    void apiStats(      cb::Event::Request &req, const cb::JSON::ValuePtr &msg);
    void apiConnections(cb::Event::Request &req, const cb::JSON::ValuePtr &msg);
    void apiHelp(       cb::Event::Request &req, const cb::JSON::ValuePtr &msg);

    // Webpage handler methods
    bool forceSecure(cb::Event::Request &req);
    bool loginPage(  cb::Event::Request &req);
  };
}
