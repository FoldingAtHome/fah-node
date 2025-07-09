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
