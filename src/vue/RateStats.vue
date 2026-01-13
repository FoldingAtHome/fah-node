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
import util from './util'


export default {
  props: ['title', 'stats'],


  methods: {
    format_rate(name, value) {
      if (name == 'pool.read' || name == 'pool.write')
        return util.human_size(value) + '/s'
      return parseFloat(value).toFixed(3) + '/s'
    },


    format_total(name, value) {
      if (name == 'pool.read' || name == 'pool.write')
        return util.human_size(value)
      return value.toLocaleString()
    }
  }
}
</script>

<template lang="pug">
.stats
  .title {{title}}
  table
    tr
      th Event
      th Rate
      th Total

    tr(v-for="(stat, key) in stats")
      td {{key.toLowerCase()}}
      td {{format_rate(key, stat.rate)}}
      td
        span(v-if="stat.total") {{format_total(key, stat.total)}}
</template>

<style lang="stylus">
#content .stats table
  th
    text-align left

  td
    font-family courier
    white-space pre

    &:nth-child(n + 2)
      text-align right
</style>
