/******************************************************************************\

                      Copyright 2017-2021. foldingathome.org
                    Copyright 2001-2017. Stanford University.
                               All Rights Reserved.

                  For information regarding this software email:
                                 Joseph Coffland
                          joseph@cauldrondevelopment.com

\******************************************************************************/

export default {
  api(method, path, data) {
    let opts = {method}
    path = '/api/' + path

    if (data) {
      if (method == 'PUT' || method == 'POST') {
        opts['headers'] = {'Content-Type': 'application/json; charset=utf-8'}
        opts['body']    = JSON.stringify(data)

      } else path += '?' + new URLSearchParams(data)
    }

    return fetch(path, opts).then(r => {
      if (!r.ok) return Promise.reject(r.statusText)
      if (method == 'GET') return r.json()
    })
  },


  get(   path, data) {return this.api('GET',    path, data)},
  put(   path, data) {return this.api('PUT',    path, data)},
  post(  path, data) {return this.api('POST',   path, data)},
  delete(path, data) {return this.api('DELETE', path, data)},
}
