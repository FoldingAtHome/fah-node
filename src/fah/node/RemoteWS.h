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

#include <cbang/ws/JSONWebsocket.h>


namespace FAH {
  namespace Node {
    class App;
    class Account;

    class RemoteWS : public cb::WS::JSONWebsocket {
    protected:
      App &app;
      std::string id;
      cb::JSON::ValuePtr login;
      cb::SmartPointer<Account> account;

    public:
      RemoteWS(App &app);
      ~RemoteWS();

      const std::string &getID() const {return id;}
      const cb::JSON::ValuePtr &getLogin() const {return login;}

      void setAccount(const cb::SmartPointer<Account> account)
        {this->account = account;}

      // Virtual interface
      virtual std::string getWSType() = 0;
      virtual void onMessage(
        const std::string &type, const cb::JSON::ValuePtr &msg) = 0;

      // From cb::WS::JSONWebsocket
      using cb::WS::JSONWebsocket::onMessage;
      void onMessage(const cb::JSON::ValuePtr &msg) override;

      // From cb::WS::Websocket
      void onShutdown() override;

    protected:
      void onLogin(const cb::JSON::ValuePtr &msg);
    };
  }
}
