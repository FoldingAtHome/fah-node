<script>
export default {
  data() {
    return {
      info: {}
    }
  },


  async mounted() {
    this.info = await this.$ws.request('info')
  }
}
</script>

<template lang="pug">
div
  h1: a(href="/api/info" target="_top") Server Info

  table
    template(v-for="(cat, key) in info")
      tr: th(colspan="2") {{key}}

      tr(v-for="(value, key) in cat")
        td {{key}}
        td {{value}}
</template>

<style lang="stylus">
</style>
