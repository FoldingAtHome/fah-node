<script>
import api from './api'


export default {
  data() {
    return {
      loading: true,
      stats:   {},
    }
  },


  mounted() {this.update()},
  unmounted() {clearTimeout(this.timer)},


  methods: {
    load_stats(stats) {
      this.stats.net  = {}
      this.stats.http = {}
      this.stats.task = {}

      for (let key in stats) {
        if (key.startsWith('HTTP_')) this.stats.http[key] = stats[key]
        else if (key.endsWith('-task'))
          this.stats.task[key.slice(0, key.length - 5)] = stats[key]
        else this.stats.net[key] = stats[key]
      }
    },


    update() {
      api.get('stats').then(data => {
        this.stats = data
        this.load_stats(data.net)

      }).finally(() => {
        this.timer = setTimeout(this.update, 2000)
        this.loading = false
      })
    }
  }
}
</script>

<template lang="pug">
.main-page
  h1(v-if="loading") Loading...
  template(v-else)
    h1: a(href="/api/stats" target="_top") Stats

    .main-stats
      RateStats( title="Network",        :stats="stats.net")
      RateStats( title="HTTP Responses", :stats="stats.http")
      RateStats( title="Tasks",          :stats="stats.task")
</template>

<style lang="stylus">
.main-page .main-stats
  display flex
  gap 1em
  justify-content center

  .stats .title
    font-weight bold
    font-size 110%
</style>
