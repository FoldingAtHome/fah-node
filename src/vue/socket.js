/******************************************************************************\

                      Copyright 2017-2021. foldingathome.org
                    Copyright 2001-2017. Stanford University.
                               All Rights Reserved.

                  For information regarding this software email:
                                 Joseph Coffland
                          joseph@cauldrondevelopment.com

\******************************************************************************/

class Socket {
  constructor() {
    this.url       = 'wss://' + location.host + '/api/ws'
    this.connected = false
    this.count     = 0
    this.pending   = {}
    this.listeners = []

    this.connect()
  }


  destroy() {
    this.connect = () => {}
    this.close()
  }


  add_listener(listener) {
    this.listeners.push(listener)
    if (this.connected) listener('open')
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
    for (let listener of this.listeners)
      listener('open')
  }


  _close(event) {
    this.connected = false
    this.ws = undefined

    let pending = this.pending
    this.pending = {}

    for (const p of Object.values(pending))
      p.reject('closed')

    for (let listener of this.listeners)
      listener('close')

    setTimeout(() => this.connect(), 2000) // Reconnect
  }


  _error(event) {}


  _message(event) {
    let msg = JSON.parse(event.data)

    if (msg.id in this.pending) {
      this.pending[msg.id].resolve(msg.data)
      delete this.pending[msg.id]

    } else
      for (let listener of this.listeners)
        listener('message', msg)
  }


  async request(name) {
    if (!this.connected) return Promise.reject('not connected')

    let id = this.count++
    this.send({request: name, id})

    return new Promise((resolve, reject) => {
      this.pending[id] = {resolve, reject}
    })
  }
}


export default Socket;
