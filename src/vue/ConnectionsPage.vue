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
import util       from './util'
import Connection from './Connection.vue'

let columns =
    'ip id status https bytes_in bytes_out rate size duration eta progress'
    .split(' ')


export default {
  data() {
    return {
      loading: true,
      columns: columns,
      connections: [],
      sort: 'rate',
      descending: true
    }
  },


  components: {Connection},


  computed: {
    sorted() {
      let key = this.sort
      let descending = this.descending

      function cmp(a, b) {
        if (a[key] < b[key]) return descending ?  1 : -1
        if (b[key] < a[key]) return descending ? -1 :  1
        return 0
      }

      return this.connections.sort(cmp)
    },


    totals() {
      let totals = {ip: 'Totals'}

      for (let i = 0; i < this.connections.length; i++) {
        let keys = Object.keys(this.connections[i])

        for (let j = 0; j < keys.length; j++) {
          let key = keys[j]

          if ('ip id status state duration eta progress'.indexOf(key) != -1)
            continue

          if (totals[key] == undefined) totals[key] = 0
          totals[key] += this.connections[i][key]
        }
      }

      return totals
    }
  },


  mounted() {this.update()},
  unmounted() {clearTimeout(this.timer)},


  methods: {
    async update() {
      try {
        this.connections = await this.$ws.request('connections')
        this.loading = false

      } catch (e) {
      } finally {
        this.timer = setTimeout(this.update, 2000)
      }
    },


    column_click(name) {
      if (name == this.sort) this.descending = !this.descending
      else {
        this.sort = name
        this.descending = true
      }
    },


    column_icon(name) {
      return 'fa-caret-' +
        (name == this.sort ? (this.descending ? 'down' : 'up') : 'right')
    }
  }
}
</script>

<template lang="pug">
div
  div
    h1: a(href="/api/connections", target="_blank")
      | {{connections.length.toLocaleString()}} Connections

    h1(v-if="loading") Loading...
    table.connections(v-else)
      tr
        th(v-for="name in columns", @click="column_click(name)").
          #[.fa(:class="column_icon(name)")] {{name}}

      tr.totals(v-if="totals", is="vue:Connection", :c="totals")
      tr(v-for="c in sorted", is="vue:Connection", :c="c")
</template>

<style lang="stylus">
#content .connections
  td
    padding 0 5px
    min-width 6em
    text-align right

    &.ip
      min-width 10em

  th
    text-align left
    cursor pointer

  .totals
    font-weight bold

    .https, .ProgressBar, span
      visibility hidden

  progress
    border 0
    height 1.25em
</style>
