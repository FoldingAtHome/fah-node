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

#include "RemoteWS.h"


namespace FAH {
  namespace Node {
    class Account;

    class ClientWS : public RemoteWS {
    public:
      using RemoteWS::RemoteWS;
      ~ClientWS();

      void closeSession(const std::string &sid);

      // From RemoteWS
      std::string getWSType() override {return "client";}
      void onMessage(
        const std::string &type, const cb::JSON::ValuePtr &msg) override;

      // From cb::WS::Websocket
      void onShutdown() override;
    };
  }
}
