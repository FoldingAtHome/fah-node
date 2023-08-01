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

#include "Account.h"
#include "AccountWS.h"
#include "ClientWS.h"

using namespace std;
using namespace cb;
using namespace FAH::Node;


const SmartPointer<AccountWS> &Account::getAccount(uint64_t id) const {
  auto it = accounts.find(id);
  if (it == accounts.end()) THROW("Account with ID '" << id << "' not found");
  return it->second;
}


const SmartPointer<ClientWS> &Account::getClient(const string &id) const {
  auto it = clients.find(id);
  if (it == clients.end()) THROW("Client with ID '" << id << "' not found");
  return it->second;
}


void Account::add(const SmartPointer<AccountWS> &account) {
  accounts[account->getConnID()] = account;

  // Send "client" messages for each registered client
  for (auto pair: clients)
    account->notify(*pair.second);
}


void Account::add(const SmartPointer<ClientWS> &client) {
  // TODO check that client is linked to this account

  clients[client->getID()] = client;

  // Notify connected accounts
  for (auto pair: accounts)
    pair.second->notify(*client);
}


void Account::remove(const AccountWS &account) {
  // TODO Send "disconnect" messages to each connected client
  accounts.erase(account.getConnID());
}


void Account::remove(const ClientWS &client) {
  // TODO Send "disconnect" message to any connected accounts
  clients.erase(client.getID());
}
