<script>
import util from './util'


export default {
  props: ['title', 'stats'],


  methods: {
    format_rate(name, value) {
      if (value.rate != undefined) value = value.rate
      if (name == 'receiving' || name == 'sending')
        return util.human_size(value) + '/s'
      return parseFloat(value).toFixed(3) + '/s'
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
      td {{key}}
      td {{format_rate(key, stat)}}
      td
        span(v-if="stat.total") {{stat.total.toLocaleString()}}
</template>

<style lang="stylus">
#content .stats table
  th
    text-align left

  td:nth-child(n + 2)
    text-align right
</style>
