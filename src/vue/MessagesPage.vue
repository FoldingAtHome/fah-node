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
        this.stats = await this.$api.request('stats')

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
