<script>
export default {
  data() {
    return {
      connected: false,
      info:  {},
      tick:  0
    }
  },


  mounted() {
    this.$ws.add_listener(this.websocket_event)
    this.clock()
  },


  unmounted() {clearTimeout(this.timer)},


  methods: {
    clock() {
      this.tick++
      this.timer = setTimeout(() => this.clock(), 250)
    },


    async websocket_event(type) {
      if (type == 'open') this.info = await this.$ws.request('server')
      this.connected = type != 'close'
    },


    async logout() {
      await fetch('/api/logout', {method: 'PUT'})
      location.href = '/'
    }
  }
}
</script>

<template lang="pug">
#root(v-cloak)
  #header
    .logo
      a(href='https://foldingathome.org/', target="_blank")
        .logo-title Folding#[span @]home
        .logo-subtitle Distributed Computing

    .header-center
      h1.version F@H Node
      strong#message

    table.server-info
      tr(v-if='connected')
        th Version
        td v{{info.version}}

      tr(v-if='connected')
        th Started
        td(:title="info.started")
          | {{$util.how_long_ago(info.started, tick)}} ago

      tr
        th Status
        td {{connected ? 'Connected' : 'Disconnected'}}

  #menu
    router-link(to="/admin/") Home
    router-link(to="/admin/messages") Messages
    router-link(to="/admin/connections") Connections
    router-link(to="/admin/server") Server
    router-link(to="/admin/help") Help
    a(@click="logout", href="#") Logout

  router-view#content
</template>

<style lang="stylus">
@import('colors')

html *
  padding 0
  margin 0

[v-cloak]
  display none !important

body
  background #d1d1d1
  font-family Lucidatypewriter

body, #root
  overflow hidden
  position fixed
  width 100vw
  height 100vh

#root
  display flex
  flex-direction column

h1, h2, h3, h4
  margin-bottom 0.5em

pre code
  display block
  margin-left 3em
  width 80%
  background #eee
  padding 0.15em

code
  padding 0.1em
  display inline-block
  background #eee
  border 1px solid #aaa

button
  cursor pointer

.clear
  clear both

.progress-bar
  overflow visible
  background-color green

#header
  display flex
  gap 10px
  background-color #134a86
  color #fff
  text-align center
  padding 0.5em
  box-shadow 0px 0px 16px #000
  text-shadow 1px 1px #000
  z-index 10

  .header-center
    flex-grow 1

    .version
      white-space nowrap

  .logo
    white-space nowrap
    text-align left

    a
      text-decoration none
      color #fff
      font-weight bold

    .logo-title
      font-size 32pt

      span
        color red
        font-size 40pt

    .logo-subtitle
      font-size 16pt
      text-transform capitalize

  .server-info
    color #fff

    td, th
      white-space: pre

    th
      text-align right
      padding-right 5px

    td
      font-family courier
      text-align left

#menu
  height 0
  width 100%
  text-align center
  z-index 10
  margin-top -1px

  > a
    background-color #134a86
    color #fff
    font-weight bold
    padding 2px 4px
    text-decoration none
    border 1px solid black
    border-top 0
    white-space nowrap
    margin 1px
    border-bottom-left-radius 4px
    border-bottom-right-radius 4px
    text-transform capitalize
    outline 0

    &:hover
      background-color #1e76d5

#content
  text-align center
  padding 40px 30px 30px 30px
  flex-grow 1
  overflow scroll

  table
    border-collapse collapse
    empty-cells show
    margin auto

    tr:nth-child(even)
      background #ddd

    td, th
      white-space nowrap
      border 1px solid #666
      padding 3px

    th
      text-align center
      white-space nowrap
      background-color slategray
      color white
      font-size 80%

    td
      text-align left

      &.progress
        padding 0

    &.config
      th
        text-align left
        font-size 110%
        padding 0.25em 0.125em

      td
        vertical-align top

      .value
        max-width 60em
        overflow hidden
        text-overflow ellipsis
        white-space pre
</style>
