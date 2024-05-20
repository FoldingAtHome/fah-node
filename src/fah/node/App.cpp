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

#include "App.h"

#include "Server.h"
#include "Account.h"

#include <cbang/Catch.h>
#include <cbang/config/Options.h>
#include <cbang/os/SystemUtilities.h>
#include <cbang/os/DirectoryWalker.h>
#include <cbang/event/Event.h>
#include <cbang/event/FDPool.h>
#include <cbang/log/Logger.h>
#include <cbang/time/Time.h>
#include <cbang/debug/Debugger.h>
#include <cbang/openssl/KeyGenPacifier.h>

#include <cstdlib>
#include <csignal>

using namespace std;
using namespace cb;
using namespace FAH::Node;

namespace FAH {
  namespace Node {
    namespace BuildInfo {
      void addBuildInfo(const char *category);
    }
  }
}


App::App() :
  ServerApplication("Folding@home Node Server", App::_hasFeature),
  base(true, true, 16), client(base, new SSLContext), account(client),
  sessionManager(options), stats(60), server(new Server(*this)) {

  // Init Debugger before threads start
  Debugger::getStackTrace();

  // Configure commandline
  cmdLine.setAllowConfigAsFirstArg(true);
  cmdLine.setAllowPositionalArgs(false);

  // Setup ACMEv2
  account.addOptions(options);
  options["acmev2-base"].setDefault(ACMEv2::letsencrypt_base);

  options.pushCategory("Logging");
  options.add("move-log-pattern", "Regular expression which matchs log files "
              "which will be periodically moved to 'move-log-dir'.")
    ->setDefault("^.*\\.((CPUtime)|(Expired))\\.\\d+\\.log$");
  options.add("move-log-rate", "Rate in seconds at which to search for a move "
              "old log files.")->setDefault(Time::SEC_PER_HOUR);
  options.add("move-log-dir", "The directory in which to store old log files."
              )->setDefault("logs");
  options.popCategory();

  // Configure logger
  options["verbosity"].setDefault(3);
  options["log"].setDefault("log.txt");
  options["log-thread-id"].setDefault(false);
  options["log-thread-prefix"].setDefault(true);
  options["log-short-level"].setDefault(true);
  options["log-rotate"].setDefault(true);
  options["log-rotate-max"].setDefault(256);
  options["log-rotate-period"].setDefault(Time::SEC_PER_DAY);
  options["log-rotate-compression"].setDefault("bzip2");

  options["session-cookie"].setDefault("fah_node_sid");

  // Enable libevent logging
  Event::Event::enableLogging(3);

  // Handle exit signals
  addSignalEvent(SIGINT);
  addSignalEvent(SIGTERM);

  // Ignore SIGPIPE
  ::signal(SIGPIPE, SIG_IGN);

  // Priority
  base.getPool().setEventPriority(3);

  // Tasks
  auto event = base.newEvent(this, &App::moveLogsEvent);
  event->setPriority(3);
  event->add(options["move-log-rate"].toDouble());
  event->activate();

  // Info
  BuildInfo::addBuildInfo(getName().c_str());
}


App::~App() {delete server;}


bool App::_hasFeature(int feature) {
  switch (feature) {
  case FEATURE_CHECK_OPEN_FILE_LIMIT:
  case FEATURE_SERVER:
  case FEATURE_INFO:
    return true;

  case FEATURE_SIGNAL_HANDLER:
    return false;

  default: return ServerApplication::_hasFeature(feature);
  }
}


const SmartPointer<Account> &App::getAccount(const string &id) {
  // Clean up old accounts
  if (accountsClean == accounts.end()) accountsClean = accounts.begin();
  if (accountsClean != accounts.end()) {
    if (accountsClean->second->isEmpty())
      accountsClean = accounts.erase(accountsClean);
    else accountsClean++;
  }

  // Get Account
  auto it = accounts.find(id);
  if (it == accounts.end()) return accounts[id] = new Account(*this, id);
  return it->second;
}


LevelDB App::getDB(const string &ns) {return db.ns(ns);}


int App::init(int argc, char *argv[]) {
  int ret = ServerApplication::init(argc, argv);
  if (ret == -1) return ret;

  // Set up outgoing client
  if (options["bind-ip"].isSet())
    client.setBindAddress(SockAddr::parse(options["bind-ip"]));
  client.setStats(SmartPointer<RateSet>::Phony(&stats));

  server->init();
  openDB();
  initCerts();            // After openDB()
  server->initHandlers(); // After initCerts()

  // Load saved sessions
  if (getDB().has("sessions")) try {
    LOG_INFO(1, "Loading saved sessions");
    sessionManager.parse(getDB().get("sessions"));
  } CATCH_ERROR;

  return 0;
}


void App::run() {
  LOG_INFO(2, "Started");

  // Main loop
  base.dispatch();

  // Clean up
  server->shutdown();

  // Save sessions
  TRY_CATCH_ERROR(getDB().set("sessions", sessionManager.toString()));

  // Close DB
  db.close();

  LOG_INFO(1, "Clean exit");
}


void App::requestExit() {
  ServerApplication::requestExit();
  base.loopBreak();
}


void App::initCerts() {
  SSLContext &sslCtx = *server->getSSLContext();

  // Load private key
  auto db = getDB();

  if (db.has("private")) privateKey.readPrivatePEM(db.get("private"));
  else {
    privateKey.generateRSA(4096, 65537,
                           new KeyGenPacifier("Generating private key"));
    db.set("private", privateKey.privateToPEMString());
  }
  sslCtx.usePrivateKey(privateKey);

  // Client key and certificate
  KeyPair webKey;
  string webChain;
  {
   if (db.has("web-key")) webKey.readPrivatePEM(db.get("web-key"));
    else {
      webKey.generateRSA(4096, 65537, new KeyGenPacifier("Generating web key"));
      db.set("web-key", webKey.privateToPEMString());
    }
    sslCtx.usePrivateKey(webKey);

    webChain = db.get("web-chain", "");
    if (!webChain.empty()) sslCtx.useCertificateChain(webChain);
  }

  // Init ACMEv2 account
  if (options["domains"].hasValue()) {
    auto cb =
      [this, &sslCtx] (ACMEv2::KeyCert &keyCert) {
        LOG_INFO(1, "Updated web certificate");
        auto &chain = keyCert.getChain();
        getDB().set("web-chain", chain.toString());
        sslCtx.useCertificateChain(chain);
      };

    string domains = options["domains"].toString();
    account.simpleInit(privateKey, webKey, domains, webChain, *server, cb);

  } else if (!webKey.isValid() || webChain.empty()) {
    SmartPointer<Certificate> cert;

    string certPEM = db.get("self-cert", "");
    if (certPEM.empty()) {
      LOG_WARNING("Generating self-signed certficate");

      string cn = "localhost";
      if (!options["http-addresses"].toStrings().empty())
        cn = options["https-addresses"].toStrings()[0];

      cert = new Certificate;
      cert->setSerial(1);
      cert->setNotBefore(0);
      cert->setNotAfter(Time::now());
      cert->setPublicKey(privateKey);
      cert->addNameEntry("C", "CA");
      cert->addNameEntry("O", "FAH Node");
      cert->addNameEntry("CN", cn);
      cert->sign(privateKey);

      db.set("self-cert", cert->toString());

    } else cert = new Certificate(certPEM);

    LOG_WARNING("Using self-signed certficate as web certificate");
    sslCtx.usePrivateKey(privateKey);
    sslCtx.clearExtraChainCertificates();
    sslCtx.useCertificate(*cert);
  }

  // Set default oauth2 redirect URL
  if (!options["http-addresses"].toStrings().empty())
    options["google-redirect-base"]
      .setDefault(string("https://") +
                  options["https-addresses"].toStrings()[0]);
}


void App::addSignalEvent(int sig) {
  auto event = ltm.addLTO(base.newSignal(sig, this, &App::signalEvent));
  event->setPriority(0);
  event->add();
}


void App::openDB() {
  // Main DB
  const char *dbPath = "node.leveldb";
  LOG_INFO(1, "Accessing " << dbPath);
  db.open(dbPath, LevelDB::CREATE_IF_MISSING);
}


void App::signalEvent(Event::Event &e, int signal, unsigned flags) {
  switch (signalCount++) {
  case 0:
    LOG_INFO(1, "Caught signal " << signal << ", requesting shutdown");
    break;

  case 1:
    LOG_INFO(1, "Caught signal " << signal << ", next signal will terminate");
    break;

  case 2:
    LOG_INFO(1, "Caught signal " << signal << ", terminating");
    exit(1);
  }

  requestExit();
}


void App::moveLogsEvent() {
  stats.event("move-logs-task");
  string dir = options["move-log-dir"];
  SystemUtilities::ensureDirectory(dir);

  DirectoryWalker walker(".", options["move-log-pattern"], 1);

  while (walker.hasNext()) {
    string filename = walker.next();
    int64_t age = Time::now() - SystemUtilities::getModificationTime(filename);
    if (age < Time::SEC_PER_MIN * 30) continue;
    SystemUtilities::rename(filename, dir + "/" + filename);
    LOG_DEBUG(3, "Moved '" << filename << "' to '" << dir << "'");
  }
}
