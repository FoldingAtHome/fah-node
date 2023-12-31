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

#include <cbang/event/Headers.h>
#include <cbang/event/FDPool.h>
#include <cbang/event/HTTPConn.h>
#include <cbang/event/FileHandler.h>
#include <cbang/event/ACLHandler.h>
#include <cbang/event/ResourceHTTPHandler.h>
#include <cbang/event/HTTPSessionHandler.h>
#include <cbang/event/HTTPOAuth2LoginHandler.h>

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
  Event::WebServer(app.getOptions(), app.getEventBase(), new SSLContext,
                   SmartPointer<HTTPHandlerFactory>::Phony(this)),
  app(app), options(app.getOptions()), googleOAuth2(app.getOptions()) {

  setLogPrefix(true);

  // Event::Server
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
    ->setType(Option::STRINGS_TYPE);
  options.popCategory();

  // Google oauth2
  options.alias("google-client-id",     "oauth2-client-id");
  options.alias("google-client-secret", "oauth2-client-secret");
  options.alias("google-redirect-base", "oauth2-redirect-base");

  // Stats logging
  setStats(SmartPointer<RateSet>::Phony(&app.getStats()));
}


Server::~Server() {}


const SmartPointer<Event::Request> &Server::add(const RequestPtr &ws) {
  return websockets[ws->getID()] = ws;
}


void Server::remove(const Event::Request &ws) {
  websockets.erase(ws.getID());
}


void Server::init(SSLContext &ctx) {
  // Set allowed SSL ciphers
  if (options["ssl-cipher-list"].hasValue() &&
      !options["ssl-cipher-list"].toString().empty())
    ctx.setCipherList(options["ssl-cipher-list"]);

  ctx.loadSystemRootCerts();

  // Dont request client certificate and set verification depth
  ctx.setVerifyPeer(false, false, 4);
}


void Server::initHandlers() {
  // API Event::HTTP request callbacks
#define ADD_HANDLER(METHODS, PATTERN, FUNC)                      \
  addMember<Server>(METHODS, PATTERN, this, &Server::FUNC)

  // Auth, order is important here
  SmartPointer<SessionManager>::Phony sessionMan(&app.getSessionManager());
  addHandler(new Event::HTTPSessionHandler(sessionMan));
  addHandler(HTTP_GET, "/login", new Event::HTTPOAuth2LoginHandler
            (app.getClient(), SmartPointer<GoogleOAuth2>::Phony(&googleOAuth2),
             sessionMan));

  // Redirect failed auth
  auto cb = [] (Event::Request &req) {
    if (String::startsWith(req.getURI().getPath(), "/api"))
      req.reply(HTTP_UNAUTHORIZED);
    else req.redirect("/login");
  };
  addHandler(new Event::ACLHandler(aclSet, cb));

  ADD_HANDLER(HTTP_ANY, "^/((login)|(admin))$", forceSecure);
  ADD_HANDLER(HTTP_GET, ".*",                   apiXFrameOptions);
  ADD_HANDLER(HTTP_GET, "/login",               loginPage);
  ADD_HANDLER(HTTP_ANY, "/api/.*",              apiCORS);
  ADD_HANDLER(HTTP_GET, "/api/server",          apiServer);
  ADD_HANDLER(HTTP_GET, "/api/info",            apiInfo);
  ADD_HANDLER(HTTP_GET, "/api/stats",           apiStats);
  ADD_HANDLER(HTTP_GET, "/api/connections",     apiConnections);
  ADD_HANDLER(HTTP_GET, "/api/help",            apiHelp);
  ADD_HANDLER(HTTP_PUT, "/api/logout",          apiLogout);
  ADD_HANDLER(HTTP_ANY, "/api/.*",              apiNotFound);

  // Add Web page handlers
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
      HTTP_GET, "/pub/.*", new Event::FileHandler(options["http-pub"], 5));
}


void Server::init() {
  // Init web server
  Event::WebServer::init();

  // Web server event priorities
  for (auto it = getPorts().begin(); it != getPorts().end(); it++)
    (*it)->setPriority((*it)->isSecure() ? 4 : 8);

  // Init SSL
  init(*getSSLContext());
  init(*app.getClient().getSSLContext());

  // Load ACL
  stringstream str;
  str << Node::resource0.get("acl.json");
  aclSet.read(str);

  // Grant configured admin access
  vector<string> admins = app.getOptions()["admins"].toStrings();
  for (unsigned i = 0; i < admins.size(); i++)
    aclSet.groupAddUser("admin", admins[i]);
}


SmartPointer<Event::Request> Server::createRequest
(Event::RequestMethod method, const URI &uri, const Version &version) {
  if (method == HTTP_GET && uri.getPath() == "/ws/client")
    return add(new ClientWS(app, uri, version));

  if (method == HTTP_GET && uri.getPath() == "/ws/account")
    return add(new AccountWS(app, uri, version));

  if (method == HTTP_GET && uri.getPath() == "/api/ws")
    return add(new APIWebsocket(app, uri, version));

  return Event::WebServer::createRequest(method, uri, version);
}


void Server::writeServer(JSON::Sink &sink) const {
  sink.beginDict();
  sink.insert("version", app.getVersion().toString());
  sink.insert("uptime", app.getUptime());
  sink.insert("time", Time().toString());
  sink.endDict();
}


void Server::writeInfo(JSON::Sink &sink) const {
  Info::instance().write(sink);
}


void Server::writeStats(JSON::Sink &sink) const {
  sink.beginDict();

  auto &base = app.getEventBase();
  sink.insertDict("net");
  sink.insert("receiving", base.getPool().getReadRate().get());
  sink.insert("sending",   base.getPool().getWriteRate().get());
  getStats()->insert(sink);
  sink.endDict();

  sink.insert("http-conns", getConnections().size());

  // Events
  sink.insertDict("events");
  sink.insert("total",  base.getNumEvents());
  sink.insert("active", base.getNumActiveEvents());

  map<int, unsigned> counts;
  base.countActiveEventsByPriority(counts);

  for (auto it = counts.begin(); it != counts.end(); it++)
    sink.insert(String::printf("priority-%u", it->first), it->second);

  sink.endDict();

  // Error & warning rates
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
    sink.insert("ip",           conn.getPeer().getHost());
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


void Server::writeHelp(JSON::Sink &sink) const {options.write(sink);}


bool Server::apiCORS(Event::Request &req) {
  Event::Headers &hdrsIn  = req.getInputHeaders();
  Event::Headers &hdrsOut = req.getOutputHeaders();

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


bool Server::apiNotFound(Event::Request &req) {
  req.sendJSONError(HTTP_NOT_FOUND, "API endpoint not found");
  return true;
}


void Server::apiLogout(Event::Request &req, const JSON::ValuePtr &msg) {
  if (req.getSession().isNull()) THROWX("Not logged in", HTTP_BAD_REQUEST);
  else app.getSessionManager().closeSession(req.getSession()->getID());
}


bool Server::apiXFrameOptions(Event::Request &req) {
  Event::Headers &hdrs = req.getOutputHeaders();
  hdrs.set("X-Frame-Options", "DENY");
  return false; // Continue processing
}


void Server::apiServer(Event::Request &req, const JSON::ValuePtr &msg) {
  writeServer(*req.getJSONWriter());
}


void Server::apiInfo(Event::Request &req, const JSON::ValuePtr &msg) {
  writeInfo(*req.getJSONWriter());
}


void Server::apiStats(Event::Request &req, const JSON::ValuePtr &msg) {
  writeStats(*req.getJSONWriter());
}


void Server::apiConnections(Event::Request &req, const JSON::ValuePtr &msg) {
  writeConnections(*req.getJSONWriter());
}


void Server::apiHelp(Event::Request &req, const JSON::ValuePtr &msg) {
  writeHelp(*req.getJSONWriter());
}


bool Server::forceSecure(Event::Request &req) {
  if (req.isSecure()) return false;

  // Redirect to first secure port
  for (auto it = getPorts().begin(); it != getPorts().end(); it++)
    if ((*it)->isSecure()) {
      const IPAddress &addr = (*it)->getAddr();

      URI redirectURI = req.getURI();
      redirectURI.setScheme("https");
      redirectURI.setHost(addr.getHost());
      redirectURI.setPort(addr.getPort());
      req.redirect(redirectURI);
      return true;
    }

  req.reply(HTTP_UNAUTHORIZED, "No secure port configured");
  return true;
}


bool Server::loginPage(Event::Request &req) {
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
