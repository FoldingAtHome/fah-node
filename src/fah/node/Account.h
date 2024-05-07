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

#include <cbang/json/Value.h>

#include <cstdint>
#include <map>


namespace FAH {
  namespace Node {
    class App;
    class AccountWS;
    class ClientWS;

    class Account : virtual public cb::RefCounted {
      App &app;
      std::string id;

      typedef cb::SmartPointer<AccountWS> AccountWSPtr;
      typedef cb::SmartPointer<ClientWS>  ClientWSPtr;
      std::map<std::string, AccountWSPtr> accounts;
      std::map<std::string, ClientWSPtr>  clients;

      std::map<std::string, cb::JSON::ValuePtr> broadcastMsgs;

    public:
      Account(App &app, const std::string &id);

      const std::string &getID() const {return id;}
      bool isEmpty() const {return accounts.empty() && clients.empty();}

      AccountWSPtr getSession(const std::string &sid) const;
      ClientWSPtr findClient(const std::string &id) const;
      void add(const AccountWSPtr &account);
      void add(const ClientWSPtr  &client);
      void removeAccount(const std::string &id);
      void removeClient(const std::string &id);

      void broadcast(const cb::JSON::ValuePtr &msg);
    };
  }
}
