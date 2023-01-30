/******************************************************************************\

                      Copyright 2017-2021. foldingathome.org
                    Copyright 2001-2017. Stanford University.
                               All Rights Reserved.

                  For information regarding this software email:
                                 Joseph Coffland
                          joseph@cauldrondevelopment.com

\******************************************************************************/

import util from './util'


export default {
  pad_num:  util.pad_num, // Needed by duration
  duration: util.human_duration,
  size:     util.human_size,
  number:   util.human_number,
  hex(value)     {return '0x' + parseInt(value).toString(16)},
  percent(value) {return (parseFloat(value) * 100).toFixed(2) + '%'},


  since(time, offset) {
    if (typeof time == 'undefined') return ''
    let t = moment(time)
    if (offset) t = t.add(offset, 'seconds')
    return t.fromNow()
  }
}
