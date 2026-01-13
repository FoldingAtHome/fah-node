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
export default {
  data() {
    return {
      loading: true,
      stats: {}
    }
  },


  computed: {
    log() {
      if (this.stats.log == undefined) return []

      let log = []
      for (let key in this.stats.log) {
        let e = this.stats.log[key]
        e.key = key
        log.push(e)
      }

      function cmp(a, b) {
        if (a.key[0] == b.key[0]) {
          if (a.total == b.total) {
            if (a.rate == b.rate)
              return (a.msg || '').localeCompare(b.msg || '')

            return b.rate - a.rate
          }

          return b.total - a.total
        }

        return a.key[0] - b.key[0]
      }

      return log.sort(cmp)
    }
  },


  mounted() {this.update()},
  unmounted() {clearTimeout(this.timer)},


  methods: {
    async update() {
      try {
        this.stats = await this.$ws.request('stats')

      } catch (e) {
      } finally {
        this.timer   = setTimeout(this.update, 2000)
        this.loading = false
      }
    }
  }
}
</script>

<template lang="pug">
#content.error-warn
  h2 Logged Errors & Warnings

  table
    tr
      th Rate
      th Count
      th Type
      th Message

    tr(v-for="e in log", :title="e.key + '\\n' + e.msg",
      :class="e.key[0] == 'E' ? 'error' : 'warning'")
      td.rate {{e.rate.toFixed(3)}}/s
      td.count {{e.total.toLocaleString()}}
      td.type {{e.key[0] == 'E' ? 'ERROR' : 'WARNING'}}
      td.message {{e.msg}}
</template>

<style lang="stylus">
@import('colors')

#content.error-warn
  th
    text-align center

  td
    text-align left

  .rate, .count
    text-align right

  .message
    max-width 30em
    text-overflow ellipsis
    overflow hidden

  .error
    error-style()

  .warning
    warn-style()
</style>
