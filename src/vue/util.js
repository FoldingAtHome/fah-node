/******************************************************************************\

                      Copyright 2017-2021. foldingathome.org
                    Copyright 2001-2017. Stanford University.
                               All Rights Reserved.

                  For information regarding this software email:
                                 Joseph Coffland
                          joseph@cauldrondevelopment.com

\******************************************************************************/

const durations = [
  [60 * 60 * 24 * 365, 'y', 0],
  [60 * 60 * 24,       'd', 3],
  [60 * 60,            'h', 2],
  [60,                 'm', 2],
  [1,                  's', 2],
]


function pad_left(x, width, pad = ' ') {
  let s = '' + x
  while (s.length < width) s = pad + s
  return s
}


function _duration(t, index, parts, force = false) {
  if (durations.length <= index || parts <= 0 || t < 0 || !isFinite(t)) return

  const [secs, unit, width] = durations[index]
  if (!force && t < secs) return _duration(t, index + 1, parts)

  let s    = Math.floor(t / secs) + unit
  let rest = _duration(t % secs, index + 1, parts - 1, true)
  if (rest) s += ' ' + pad_left(rest, width + 1)

  return s
}


export default {
  how_long_ago(t) {
    let d = (new Date().getTime() - new Date(t).getTime()) / 1000
    return this.human_duration(d)
  },


  human_duration(t, parts = 2) {
    if (t == Infinity) return t
    if (t < 0 || !isFinite(t)) return 'unknown'
    return _duration(t, 0, parts)
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
