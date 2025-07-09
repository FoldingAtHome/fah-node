<script>
const READ           = 1 << 0
const WRITE          = 1 << 1
const CLOSE          = 1 << 2
const TIMEOUT        = 1 << 3
const READ_CLOSED    = 1 << 4
const WRITE_CLOSED   = 1 << 5
const READ_TIMEDOUT  = 1 << 6
const WRITE_TIMEDOUT = 1 << 7


export default {
  name: 'Connection',
  props: ['c'],

  computed: {
    status() {
      let s = this.c.status
      let l = []

      if (s & READ)           l.push('R')
      if (s & WRITE)          l.push('W')
      if (s & CLOSE)          l.push('C')
      if (s & TIMEOUT)        l.push('T')
      if (s & READ_CLOSED)    l.push('RC')
      if (s & WRITE_CLOSED)   l.push('WC')
      if (s & READ_TIMEDOUT)  l.push('RT')
      if (s & WRITE_TIMEDOUT) l.push('WT')

      return l.join(',')
    }
  }
}
</script>

<template lang="pug">
tr
  td.ip             {{c.ip}}
  td.id             {{c.id}}
  td.status         {{status}}
  td.https          {{c.https ? 'yes' : 'no'}}
  td.bytes_in       {{$util.human_size(c.bytes_in)}}
  td.bytes_out      {{$util.human_size(c.bytes_out)}}
  td.rate           {{$util.human_size(c.rate)}}/s
  td.length         {{$util.human_size(c.length)}}
  td.duration: span {{$util.human_duration(c.duration)}}
  td.eta:      span {{$util.human_duration(c.eta)}}
  td.progress: ProgressBar(v-if="c.progress", :value="c.progress")
</template>

<style lang="stylus">
</style>
