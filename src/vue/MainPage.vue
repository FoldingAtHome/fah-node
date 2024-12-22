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
      loading: true,
      stats:   {},
      chart_opts: {
        animation: {duration: 0},
        scales: {x: {reverse: true}}
      }
    }
  },


  components: {Line},


  mounted() {this.update()},
  unmounted() {clearTimeout(this.timer)},


  methods: {
    async update() {
      try {
        let data   = await this.$api.request('stats')
        this.stats = data
        this.loading = false
      } catch (e) {
        return
      } finally {
        this.timer = setTimeout(this.update, 2000)
      }

      this.update_chart()
    },


    update_chart() {
      let data = this.stats.rate_log
      let datasets = []

      for (const [k, v] of Object.entries(data.measurements))
        datasets.push({label: k, data: v.rates})

      this.chart_data = {datasets, labels: data.times}
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
      RateStats(v-if="Object.keys(stats.rates).length",
        title="Event Rates", :stats="stats.rates")

      Line(v-if="stats.rate_log.times.length",
        :data="chart_data", :options="chart_opts")
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
