<script>
export default {
  props: ['title', 'stats']
}
</script>

<template lang="pug">
.stats
  .title {{title}}
  table
    tr(v-for="(stat, key) in stats")
      th {{key}}
      td {{stat}}
</template>

<style lang="stylus">
</style>
