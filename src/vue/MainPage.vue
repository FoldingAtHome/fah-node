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
import {
  Chart,
  Title,
  Tooltip,
  Legend,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Colors
} from 'chart.js'
import {Line} from 'vue-chartjs'

Chart.register(
  CategoryScale, LinearScale, Title, Tooltip, Legend, PointElement, LineElement,
  Colors)


export default {
  data() {
    return {
      loading:    true,
      stats:      {},
      rate_log:   {},
      chart_data: {},
      chart_opts: {
        animation: {duration: 0},
        scales: {x: {reverse: true}}
      },
    }
  },


  components: {Line},


  mounted() {
    this.$ws.add_listener(this.websocket_event)
    this.update()
  },


  unmounted() {clearTimeout(this.timer)},


  methods: {
    websocket_event(type, msg) {
      if (type != 'message') return

      if (msg.type == 'rate-log-init') {
        this.rate_log = msg.data
        this.update_chart()
      }

      if (msg.type == 'rate') {
        const [time, key, rate, total] = msg.data
        let log = this.rate_log

        if (!(key in log.measurements)) {
          let rates  = []
          let totals = []

          for (let i = 0; i < log.times.length; i++) {
            rates.push(null)
            totals.push(null)
          }

          log.measurements[key] = {rates, totals}
        }

        let m = log.measurements[key]
        log.times.push(time)
        m.rates.push(rate)
        m.totals.push(total)

        this.chart_dirty = true
      }
    },


    async update() {
      try {
        this.stats   = await this.$ws.request('stats')
        this.loading = false
      } catch (e) {}

      this.timer = setTimeout(this.update, 2000)

      if (this.chart_dirty) this.update_chart()
    },


    update_chart(data) {
      let datasets = []

      for (const [k, v] of Object.entries(data.measurements || {}))
        datasets.push({label: k, data: v.rates})

      this.chart_data = {datasets, labels: data.times}
      this.chart_dirty = false
    }
  }
}
</script>

<template lang="pug">
.main-page
  h1(v-if="loading") Loading...
  template(v-else)
    h1: a(href="/api/stats" target="_top") Stats

    .connections
      | {{(stats.connections || 0).toLocaleString()}} Active Connections
    .main-stats
      RateStats(v-if="Object.keys(stats.rates || {}).length",
        title="Event Rates", :stats="stats.rates")

      Line(v-if="chart_data.datasets", :data="chart_data",
        :options="chart_opts")
</template>

<style lang="stylus">
.main-page
  display flex
  flex-direction column
  gap 0.5em

  .connections
    font-weight bold

  .main-stats
    display flex
    gap 1em
    justify-content center
    flex-direction column

    .stats .title
      font-weight bold
      font-size 110%
</style>
