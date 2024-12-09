<script>
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
    async update() {
      try {
        let data = await this.$api.request('stats')
        this.stats = data

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
.main-page
  h1(v-if="loading") Loading...
  template(v-else)
    h1: a(href="/api/stats" target="_top") Stats

    .connections {{stats.connections.toLocaleString()}} Active Connections
    .main-stats
      template(v-for="(rates, name) of stats.rates")
        RateStats(v-if="Object.keys(rates).length",
          :title="name", :stats="rates")
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

    .stats .title
      font-weight bold
      font-size 110%
</style>
