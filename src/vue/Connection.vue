<!--

                   This file is part of FoldingAtHome/fah-node

        The fah-node enables secure remote access to Folding@home clients.
                    Copyright (c) 2023-2026, foldingathome.org
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

-->

<script>
const READ           = 1 << 0
const WRITE          = 1 << 1
const CLOSE          = 1 << 2
const TIMEOUT        = 1 << 3
const READ_CLOSED    = 1 << 4
const WRITE_CLOSED   = 1 << 5
const READ_TIMEDOUT  = 1 << 6
const WRITE_TIMEDOUT = 1 << 7


export default {
  name: 'Connection',
  props: ['c'],

  computed: {
    status() {
      let s = this.c.status
      let l = []

      if (s & READ)           l.push('R')
      if (s & WRITE)          l.push('W')
      if (s & CLOSE)          l.push('C')
      if (s & TIMEOUT)        l.push('T')
      if (s & READ_CLOSED)    l.push('RC')
      if (s & WRITE_CLOSED)   l.push('WC')
      if (s & READ_TIMEDOUT)  l.push('RT')
      if (s & WRITE_TIMEDOUT) l.push('WT')

      return l.join(',')
    }
  }
}
</script>

<template lang="pug">
tr
  td.ip             {{c.ip}}
  td.id             {{c.id}}
  td.status         {{status}}
  td.https          {{c.https ? 'yes' : 'no'}}
  td.bytes_in       {{$util.human_size(c.bytes_in)}}
  td.bytes_out      {{$util.human_size(c.bytes_out)}}
  td.rate           {{$util.human_size(c.rate)}}/s
  td.length         {{$util.human_size(c.length)}}
  td.duration: span {{$util.human_duration(c.duration)}}
  td.eta:      span {{$util.human_duration(c.eta)}}
  td.progress: ProgressBar(v-if="c.progress", :value="c.progress")
</template>

<style lang="stylus">
</style>
