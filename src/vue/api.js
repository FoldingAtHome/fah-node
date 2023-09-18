/******************************************************************************\

                      Copyright 2017-2021. foldingathome.org
                    Copyright 2001-2017. Stanford University.
                               All Rights Reserved.

                  For information regarding this software email:
                                 Joseph Coffland
                          joseph@cauldrondevelopment.com

\******************************************************************************/

class API {
  constructor() {
    this.url = 'wss://' + location.host + '/api/ws'
    this.connected = false
    this.connect()
    this.count = 0
    this.pending = {}
  }


  destroy() {
    this.connect = () => {}
    this.close()
  }


  close() {if (this.ws) this.ws.close()}


  connect() {
    if (this.ws != undefined) return

    console.debug('Connecting to ' + this.url)

    this.ws = new WebSocket(this.url)

    this.ws.onopen    = e => this._open(e)
    this.ws.onclose   = e => this._close(e)
    this.ws.onerror   = e => this._error(e)
    this.ws.onmessage = e => this._message(e)
  }


  send(msg) {
    if (this.connected) this.ws.send(JSON.stringify(msg))
    else console.debug('Cannot send message, not connected:', msg)
  }


  _open(event) {
    this.connected = true
  }


  _close(event) {
    this.connected = false
    this.ws = undefined

    let pending = this.pending
    this.pending = {}

    for (const p of Object.values(pending))
      p.reject('closed')

    this.connect() // Reconnect
  }


  _error(event) {}


  _message(event) {
    let msg = JSON.parse(event.data)
    this.pending[msg.id].resolve(msg.data)
    delete this.pending[msg.id]
  }


  async request(name) {
    if (!this.connected) return Promise.reject('not connected')

    let id = this.count++
    this.send({request: name, id})

    return new Promise((resolve, reject) => {
      this.pending[id] = {resolve, reject}
    })
  }


  async api(method, path, data) {
    let opts = {method}
    path = '/api/' + path

    if (data) {
      if (method == 'PUT' || method == 'POST') {
        opts['headers'] = {'Content-Type': 'application/json; charset=utf-8'}
        opts['body']    = JSON.stringify(data)

      } else path += '?' + new URLSearchParams(data)
    }

    let r = await fetch(path, opts)
    if (r.status == 401) {
      location.href = '/login'
      throw 'Unauthorized'
    }
    if (!r.ok) return Promise.reject(r.statusText)
    if (method == 'GET') return r.json()
  }


  async get(   path, data) {return this.api('GET',    path, data)}
  async put(   path, data) {return this.api('PUT',    path, data)}
  async post(  path, data) {return this.api('POST',   path, data)}
  async delete(path, data) {return this.api('DELETE', path, data)}
}


export default API;
