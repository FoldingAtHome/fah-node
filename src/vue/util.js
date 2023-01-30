/******************************************************************************\

                      Copyright 2017-2021. foldingathome.org
                    Copyright 2001-2017. Stanford University.
                               All Rights Reserved.

                  For information regarding this software email:
                                 Joseph Coffland
                          joseph@cauldrondevelopment.com

\******************************************************************************/


export default {
  pad_num(n, width) {
    let s = '' + n

    for (let i = 1; i < width; i++) {
      if (n < 10) s = '0' + s
      else n = Math.floor(n / 10)
    }

    return s
  },


  human_duration(t) {
    if (t == undefined) return 'unknown'
    if (t == Infinity)  return t

    let s = ''

    let sec  = Math.floor(t % 60); t = Math.floor(t / 60)
    let min  = t % 60;  t = Math.floor(t / 60)
    let hour = t % 24;  t = Math.floor(t / 24)
    let day  = t % 365; t = Math.floor(t / 365)

    if (t)                    s += t + 'y '
    if (day)                  s += day + 'd '
    if (!t    && hour)        s += this.pad_num(hour, s ? 2 : 0) + 'h '
    if (!day  && min)         s += this.pad_num(min,  s ? 2 : 0) + 'm '
    if (!hour && (sec || !s)) s += this.pad_num(sec,  s ? 2 : 0) + 's'

    return s.trim()
  },


  human_size(bytes, precision) {
    if (typeof bytes != 'number' || isNaN(bytes)) return 'unknown'
    if (typeof precision == 'undefined') precision = 1

    let i
    for (i = 0; 1024 < bytes && i < 4; i++) bytes /= 1024

    bytes = bytes.toFixed(precision)

    return bytes + ['B', 'KiB', 'MiB', 'GiB', 'TiB'][i]
  },


  human_number(bytes, precision) {
    if (typeof bytes != 'number' || isNaN(bytes)) return 'unknown'
    if (typeof precision == 'undefined') precision = 0

    let i
    for (i = 0; 1024 < bytes && i < 4; i++) bytes /= 1024

    bytes = bytes.toFixed(precision)

    return Number(bytes) + ['', 'K', 'M', 'G', 'T'][i]
  }
}
