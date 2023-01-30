<script>
import util from './util'


export default {
  props: ['title', 'stats'],


  methods: {
    format(name, value) {
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
    tr(v-for="(stat, key) in stats")
      th {{key}}
      td {{format(key, stat)}}
</template>

<style lang="stylus">
</style>
