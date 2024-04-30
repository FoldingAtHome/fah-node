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

#include <cbang/ServerApplication.h>
#include <cbang/util/RateSet.h>
#include <cbang/db/LevelDB.h>
#include <cbang/json/Sink.h>
#include <cbang/acmev2/Account.h>
#include <cbang/event/Base.h>
#include <cbang/event/Event.h>
#include <cbang/http/Client.h>
#include <cbang/http/SessionManager.h>
#include <cbang/openssl/Certificate.h>
#include <cbang/openssl/SSLContext.h>
#include <cbang/openssl/KeyPair.h>


#include <ostream>
#include <string>
#include <vector>
#include <map>


namespace cb {namespace Event {class Event;}}

namespace FAH {
  namespace Node {
    class Server;
    class Account;

    class App : public cb::ServerApplication {
    protected:
      cb::Event::Base base;
      cb::HTTP::Client client;
      cb::KeyPair privateKey;
      cb::ACMEv2::Account account;
      cb::HTTP::SessionManager sessionManager;
      cb::RateSet stats;

      Server *server;
      typedef std::map<std::string, cb::SmartPointer<Account> > accounts_t;
      accounts_t accounts;
      accounts_t::iterator accountsClean = accounts.end();

      cb::LevelDB db;

      unsigned signalCount = 0;

      std::map<std::string, cb::Event::EventPtr> events;

    public:
      App();
      ~App();

      static bool _hasFeature(int feature);

      cb::Event::Base    &getEventBase()            {return base;}
      cb::HTTP::Client   &getClient()               {return client;}
      cb::HTTP::SessionManager &getSessionManager() {return sessionManager;}
      cb::RateSet        &getStats()                {return stats;}
      Server             &getServer()               {return *server;}

      const cb::SmartPointer<Account> &getAccount(const std::string &id);

      cb::LevelDB getDB(const std::string &ns = "config:");

      // From Application
      int init(int argc, char *argv[]);
      void run(); ///< Start the node server
      void requestExit();

    private:
      void initCerts();
      cb::Event::EventPtr addSignalEvent(int sig);
      void openDB();
      void signalEvent(cb::Event::Event &e, int signal, unsigned flags);
      void moveLogsEvent();
    };
  }
}
