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
#include "App.h"

#include <cbang/Catch.h>
#include <cbang/log/Logger.h>
#include <cbang/json/Reader.h>


using namespace std;
using namespace cb;
using namespace FAH::Node;


Account::Account(App &app, const string &id) : app(app), id(id) {
  // Load broadcasts
  auto db = app.getDB(id + ":broadcast:");

  for (auto it = db.first(); it.valid(); it++)
    try {
      broadcastMsgs[it.key()] = JSON::Reader::parseString(it.value());
    } CATCH_ERROR;
}


SmartPointer<AccountWS> Account::getSession(const string &sid) const {
  auto it = accounts.find(sid);
  return it == accounts.end() ? 0 : it->second;
}


const SmartPointer<ClientWS> &Account::getClient(const string &id) const {
  auto it = clients.find(id);
  if (it == clients.end()) THROW("Client with ID '" << id << "' not found");
  return it->second;
}


void Account::add(const SmartPointer<AccountWS> &account) {
  accounts[account->getSessionID()] = account;

  // Send "client" messages for each logged in client
  for (auto p: clients)
    account->connected(*p.second);

  // Send latest broadcasts
  for (auto p: broadcastMsgs)
    account->send(*p.second);
}


void Account::add(const SmartPointer<ClientWS> &client) {
  // TODO check that client is linked to this account

  clients[client->getID()] = client;

  // Notify connected accounts
  for (auto p: accounts)
    p.second->connected(*client);

  // Send latest broadcasts
  for (auto p: broadcastMsgs)
    client->send(*p.second);
}


void Account::remove(const AccountWS &account) {
  string sid = account.getSessionID();
  accounts.erase(sid);

  // Send "session-close" message to any connected clients
  for (auto p: clients)
    p.second->closeSession(sid);
}


void Account::remove(const ClientWS &client) {
  clients.erase(client.getID());

  // Send "disconnect" message to any connected accounts
  for (auto p: accounts)
    p.second->disconnected(client);
}


void Account::broadcast(const JSON::ValuePtr &msg) {
  string cmd = msg->selectString("payload.cmd");
  LOG_DEBUG(3, "Account " << getID() << " broadcast " << cmd);

  // Check timestamp
  auto it = broadcastMsgs.find(cmd);
  if (it != broadcastMsgs.end()) {
    auto current = it->second;
    uint64_t newTS = Time(    msg->selectString("payload.time"));
    uint64_t oldTS = Time(current->selectString("payload.time"));
    if (newTS <= oldTS) return;
  }

  // Save broadcast
  auto db = app.getDB(getID() + ":broadcast:");
  db.set(cmd, msg->toString(0, true));
  broadcastMsgs[cmd] = msg;

  // Broadcast
  for (auto p: clients)
    p.second->send(*msg);

  for (auto p: accounts)
    p.second->send(*msg);
}
