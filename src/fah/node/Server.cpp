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

#include "Server.h"
#include "App.h"
#include "Account.h"
#include "ClientWS.h"
#include "AccountWS.h"
#include "APIWebsocket.h"

#include <cbang/Exception.h>
#include <cbang/Catch.h>
#include <cbang/String.h>
#include <cbang/Info.h>

#include <cbang/util/Resource.h>
#include <cbang/util/RateSet.h>
#include <cbang/util/RateCollectionNS.h>

#include <cbang/http/Headers.h>
#include <cbang/http/Conn.h>
#include <cbang/http/FileHandler.h>
#include <cbang/http/ACLHandler.h>
#include <cbang/http/ResourceHandler.h>
#include <cbang/http/SessionHandler.h>

#include <cbang/oauth2/LoginHandler.h>
#include <cbang/event/FDPool.h>
#include <cbang/openssl/SSLContext.h>
#include <cbang/log/Logger.h>
#include <cbang/config/Options.h>
#include <cbang/time/Time.h>


using namespace std;
using namespace cb;
using namespace FAH::Node;

namespace FAH {
  namespace Node {
    extern const DirectoryResource resource0;
  }
}


Server::Server(App &app) :
  HTTP::Server(app.getEventBase(), new SSLContext), app(app) {
  addOptions(app.getOptions());

  setLogPrefix(true);
  setPortPriority(8);
  setSecurePortPriority(4);

  // HTTP::Server
  auto &options = app.getOptions();
  options["http-addresses" ].setDefault("0.0.0.0:8080");
  options["https-addresses"].setDefault("0.0.0.0:8084");

  // Don't load these from files
  options["certificate-file"].setDeprecated();
  options["private-key-file"].setDeprecated();

  // Node Server
  options.pushCategory("Node Server");
  options.add("domains", "A list of domains for the Web server certificate.");
  options.add("http-root", "Serve web pages from this directory.  Overrides "
              "the built-in pages.");
  options.add("http-pub", "Server Web pages from disk under /pub.");

  options["http-max-body-size"   ].setDefault(1024 * 1024);
  options["http-max-headers-size"].setDefault(16 * 1024);
  options["http-timeout"         ].setDefault(90);
  options["http-max-connections" ].setDefault(4096);
  options["http-max-ttl"         ].setDefault(Time::SEC_PER_MIN);

  options.add("bind-ip", "The address to bind outgoing connections to.");
  options.add("ssl-cipher-list", "Allowed OpenSSL ciphers")
    ->setDefault("HIGH:!aNULL:!PSK:!SRP:!MD5:!RC4");
  options.add("admins", "Grant admin access to these email addresses")
    ->setType(Option::TYPE_STRINGS);
  options.popCategory();

  // Google oauth2
  googleOAuth2.addOptions(options);
  options.alias("google-client-id",     "oauth2-client-id");
  options.alias("google-client-secret", "oauth2-client-secret");
  options.alias("google-redirect-base", "oauth2-redirect-base");

  // HTTP rate stats
  auto &stats = app.getStats();
  setStats(stats->getNS("net."));
  app.getEventBase().getPool().setStats(stats->getNS("pool."));
}


Server::~Server() {}


const Server::WebsocketPtr &Server::add(const WebsocketPtr &ws) {
  return websockets[ws->getID()] = ws;
}


void Server::remove(const WS::Websocket &ws) {websockets.erase(ws.getID());}


void Server::initSSL(SSLContext &ctx) {
  // Set allowed SSL ciphers
  auto &options = app.getOptions();
  if (options["ssl-cipher-list"].hasValue() &&
      !options["ssl-cipher-list"].toString().empty())
    ctx.setCipherList(options["ssl-cipher-list"]);

  ctx.loadSystemRootCerts();
}


void Server::initHandlers() {
  // Auth, order is important here
  auto sessionMan = PhonyPtr(&app.getSessionManager());
  addHandler(new HTTP::SessionHandler(sessionMan));
  addHandler(HTTP_GET, "/login", new OAuth2::LoginHandler
    (app.getClient(), PhonyPtr(&googleOAuth2), sessionMan));

  // Redirect failed auth
  auto cb = [] (HTTP::Request &req) {
    if (String::startsWith(req.getURI().getPath(), "/api"))
      req.reply(HTTP_UNAUTHORIZED);
    else req.redirect("/login");
  };
  addHandler(new HTTP::ACLHandler(aclSet, cb));

  // API Event::HTTP request callbacks
#define ADD_MEMBER(METHODS, PATTERN, FUNC)                      \
  addMember<Server>(METHODS, PATTERN, this, &Server::FUNC)

  ADD_MEMBER(HTTP_ANY, "^/((login)|(admin))$", forceSecure);
  ADD_MEMBER(HTTP_GET, ".*",                   apiXFrameOptions);
  ADD_MEMBER(HTTP_GET, "/login",               loginPage);
  ADD_MEMBER(HTTP_GET, "/ws/client",           websocket);
  ADD_MEMBER(HTTP_GET, "/ws/account",          websocket);
  ADD_MEMBER(HTTP_GET, "/api/ws",              websocket);
  ADD_MEMBER(HTTP_ANY, "/api/.*",              apiCORS);
  ADD_MEMBER(HTTP_GET, "/api/server",          apiServer);
  ADD_MEMBER(HTTP_GET, "/api/info",            apiInfo);
  ADD_MEMBER(HTTP_GET, "/api/stats",           apiStats);
  ADD_MEMBER(HTTP_GET, "/api/connections",     apiConnections);
  ADD_MEMBER(HTTP_GET, "/api/help",            apiHelp);
  ADD_MEMBER(HTTP_PUT, "/api/logout",          apiLogout);
  ADD_MEMBER(HTTP_ANY, "/api/.*",              apiNotFound);

  // Add Web page handlers
  auto &options = app.getOptions();
  if (options["http-root"].hasValue()) {
    string root = options["http-root"];

    addHandler("/", root + "/index.html");
    addHandler("/admin.*", root + "/admin.html");
    addHandler("/login",   root + "/login.html");
    addHandler(root);

  } else {
    addHandler("/", Node::resource0.get("http/index.html"));
    addHandler("/admin.*", Node::resource0.get("http/admin.html"));
    addHandler("/login",   Node::resource0.get("http/login.html"));
    addHandler(Node::resource0.get("http"));
  }

  if (options["http-pub"].hasValue())
    addHandler(
      HTTP_GET, "/pub/.*", new HTTP::FileHandler(options["http-pub"], 5));
}


void Server::init() {
  // Init HTTP server
  HTTP::Server::init(app.getOptions());

  // Init SSL
  initSSL(*getSSLContext());
  initSSL(*app.getClient().getSSLContext());

  // Load ACL
  stringstream str;
  str << Node::resource0.get("acl.json");
  aclSet.read(str);

  // Grant configured admin access
  vector<string> admins = app.getOptions()["admins"].toStrings();
  for (unsigned i = 0; i < admins.size(); i++)
    aclSet.groupAddUser("admin", admins[i]);
}


void Server::writeServer(JSON::Sink &sink) const {
  sink.beginDict();
  sink.insert("version", app.getVersion().toString());
  sink.insert("started", Time(app.getStartTime()).toString());
  sink.endDict();
}


void Server::writeInfo(JSON::Sink &sink) const {
  Info::instance().write(sink);
}


void Server::writeStats(JSON::Sink &sink) const {
  sink.beginDict();

  // Connection count
  sink.insert("connections", getConnections().size());

  // Rates
  sink.insertDict("rates");
  app.getStats()->insert(sink, true);
  sink.endDict();

  sink.insertDict("rate_log");
  //app.getRateTracker()->insert(sink); TODO
  sink.endDict();

  // Log errors & warnings
  sink.beginInsert("log");
  Logger::instance().writeRates(sink);

  sink.endDict();
}


void Server::writeConnections(JSON::Sink &sink) const {
  sink.beginList();

  uint64_t now = Time::now();

  auto &conns = getConnections();
  for (auto it = conns.begin(); it != conns.end(); it++) {
    auto &conn = **it;
    auto readProgress  = conn.getReadProgress();
    auto writeProgress = conn.getWriteProgress();

    readProgress.setEnd(now);
    writeProgress.setEnd(now);

    bool in = writeProgress.getSize() < readProgress.getSize();
    auto progress = in ? readProgress : writeProgress;

    sink.appendDict();
    sink.insert("ip",           conn.getPeerAddr().toString(false));
    sink.insert("id",           conn.getID());
    sink.insert("status",       conn.getStatus());
    sink.insertBoolean("https", conn.isSecure());
    sink.insertBoolean("in",    in);
    sink.insert("bytes_in",     readProgress.getTotal());
    sink.insert("bytes_out",    writeProgress.getTotal());
    sink.insert("rate",         progress.getRate());
    sink.insert("rate_in",      readProgress.getRate());
    sink.insert("rate_out",     writeProgress.getRate());
    sink.insert("duration",     now - conn.getStart());
    sink.insert("length",       progress.getSize());
    sink.insert("eta",          progress.getETA());
    sink.insert("progress",     progress.getProgress());
    sink.endDict();
  }

  sink.endList();
}


void Server::writeHelp(JSON::Sink &sink) const {app.getOptions().write(sink);}


bool Server::apiCORS(HTTP::Request &req) {
  HTTP::Headers &hdrsIn  = req.getInputHeaders();
  HTTP::Headers &hdrsOut = req.getOutputHeaders();

  hdrsOut.set("Access-Control-Allow-Origin", "*");
  hdrsOut.set("Access-Control-Allow-Methods", "POST, GET, OPTIONS");

  if (hdrsIn.has("Access-Control-Request-Headers"))
    hdrsOut.set("Access-Control-Allow-Headers",
                hdrsIn.get("Access-Control-Request-Headers"));

  if (req.getMethod() == HTTP_OPTIONS) {
    req.reply();
    return true;
  }

  return false; // Continue processing
}


bool Server::websocket(HTTP::Request &req) {
  SmartPointer<WS::Websocket> ws;
  auto &path = req.getURI().getPath();

  if (path == "/ws/client")  ws = new ClientWS(app);
  if (path == "/ws/account") ws = new AccountWS(app);
  if (path == "/api/ws")     ws = new APIWebsocket(app);

  ws->upgrade(req);
  add(ws);

  return true;
}


bool Server::apiNotFound(HTTP::Request &req) {
  req.sendJSONError(HTTP_NOT_FOUND, "API endpoint not found");
  return true;
}


void Server::apiLogout(HTTP::Request &req, const JSON::ValuePtr &msg) {
  if (req.getSession().isNull()) THROWX("Not logged in", HTTP_BAD_REQUEST);
  else app.getSessionManager().closeSession(req.getSession()->getID());
}


bool Server::apiXFrameOptions(HTTP::Request &req) {
  HTTP::Headers &hdrs = req.getOutputHeaders();
  hdrs.set("X-Frame-Options", "DENY");
  return false; // Continue processing
}


void Server::apiServer(HTTP::Request &req, const JSON::ValuePtr &msg) {
  req.send([&] (JSON::Sink &sink) {writeServer(sink);});
}


void Server::apiInfo(HTTP::Request &req, const JSON::ValuePtr &msg) {
  req.send([&] (JSON::Sink &sink) {writeInfo(sink);});
}


void Server::apiStats(HTTP::Request &req, const JSON::ValuePtr &msg) {
  req.send([&] (JSON::Sink &sink) {writeStats(sink);});
}


void Server::apiConnections(HTTP::Request &req, const JSON::ValuePtr &msg) {
  req.send([&] (JSON::Sink &sink) {writeConnections(sink);});
}


void Server::apiHelp(HTTP::Request &req, const JSON::ValuePtr &msg) {
  req.send([&] (JSON::Sink &sink) {writeHelp(sink);});
}


bool Server::forceSecure(HTTP::Request &req) {
  if (req.isSecure()) return false;
  req.reply(HTTP_UNAUTHORIZED);
  return true;
}


bool Server::loginPage(HTTP::Request &req) {
  // Get user name
  string user = req.getUser();

  // Redirect to the admin page if the user has access
  if (aclSet.allow("/admin", user)) {
    req.redirect("/admin");
    return true;
  }

  // Put user name in query string
  if (req.getURI().getQuery() != user) {
    req.redirect(string("/login?") + user);
    return true;
  }

  return false;
}
