<script>
import api        from './api'
import util       from './util'
import Connection from './Connection.vue'

let columns =
    'ip id status https bytes_in bytes_out rate size duration eta progress'
    .split(' ')


export default {
  data() {
    return {
      loading: true,
      columns: columns,
      connections: [],
      sort: 'rate',
      descending: true
    }
  },


  components: {Connection},


  computed: {
    sorted() {
      let key = this.sort
      let descending = this.descending

      function cmp(a, b) {
        if (a[key] < b[key]) return descending ?  1 : -1
        if (b[key] < a[key]) return descending ? -1 :  1
        return 0
      }

      return this.connections.sort(cmp)
    },


    totals() {
      let totals = {ip: 'Totals'}

      for (let i = 0; i < this.connections.length; i++) {
        let keys = Object.keys(this.connections[i])

        for (let j = 0; j < keys.length; j++) {
          let key = keys[j]

          if ('ip id status state duration eta progress'.indexOf(key) != -1)
            continue

          if (totals[key] == undefined) totals[key] = 0
          totals[key] += this.connections[i][key]
        }
      }

      return totals
    }
  },


  mounted() {this.update()},
  unmounted() {clearTimeout(this.timer)},


  methods: {
    update() {
      api.get('connections').then(data => {
        this.connections = data
        this.loading = false

      }).finally(() => this.timer = setTimeout(this.update, 2000))
    },


    column_click(name) {
      if (name == this.sort) this.descending = !this.descending
      else {
        this.sort = name
        this.descending = true
      }
    },


    column_icon(name) {
      return 'fa-caret-' +
        (name == this.sort ? (this.descending ? 'down' : 'up') : 'right')
    }
  }
}
</script>

<template lang="pug">
div
  div
    h1: a(href="/api/connections", target="_blank") Connections

    h1(v-if="loading") Loading...
    table.connections(v-else)
      tr
        th(v-for="name in columns", @click="column_click(name)").
          #[.fa(:class="column_icon(name)")] {{name}}

      tr.totals(v-if="totals", is="vue:Connection", :c="totals")
      tr(v-for="c in sorted", is="vue:Connection", :c="c")
</template>

<style lang="stylus">
#content .connections
  td
    padding 0 5px
    min-width 6em
    text-align right

    &.ip
      min-width 10em

  th
    cursor pointer

  tr:nth-child(even)
    background #ddd

  .totals
    font-weight bold

    .https, .ProgressBar, span
      visibility hidden

  progress
    border 0
    height 1.25em
</style>
