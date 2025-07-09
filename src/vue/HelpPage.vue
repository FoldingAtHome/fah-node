<script>
export default {
  data() {
    return {
      help: {}
    }
  },


  async mounted() {
    this.help = await this.$ws.request('help')
    this.$nextTick(() => location.hash = location.hash)
  }
}
</script>

<template lang="pug">
.help-page
  .help-menu
    h1 Menu
    ul
      li(v-for="(category, cname) in help",
        @click="category.show = !category.show")
        .fa.fa-plus
        a(:href="'#option-category-' + cname")
          | {{cname || 'Uncategorized'}}

        ul(v-if="category.show", @click.stop="")
          li(v-for="(option, name) in category")
            .fa.fa-square
            a(:href="'#option-' + name") {{name}}

  .help-content
    .options
      .option-category(v-for="(category, cname) in help",
        :id="'option-category-' + cname")
        .option-category-name {{cname}}

        .option(v-for="(option, name) in category", :id="'option-' + name")
          span.name {{name}}

          span.type.
            {{(option.optional ? '[' : '<') + option.type +
            (option.default ? ' = ' : '')
            }}<span class="default" v-if="option.default">{{
            (option.type.indexOf('string') === 0 ? '\"' : '') +
            option.default +
            (option.type.indexOf('string') === 0 ? '\"' : '')
            }}</span>{{(option.optional ? ']' : '>')}}

          .help {{option.help}}

          .constraint(v-if="option.constraint")
            | <strong>Constraints:</strong> {{option.constraint}}.
</template>

<style lang="stylus">
#content.help-page
  text-align left
  display flex
  padding 0
  overflow hidden
  height 100%

  .help-menu, .help-content
    overflow-y auto
    height calc(100% - 70px)
    margin 0
    padding 40px 10px 30px 10px

  .help-menu
    width 40em
    border-right 1px solid #aaa

    .fa
      margin-right 0.25em
      font-size 8pt

    ul
      margin-left 1.5em
      list-style none

    > ul
      padding 1em 0
      margin 0

    li
      margin 0.25em

  .help-content
    h1
      text-transform capitalize

  .title
    font-size 32pt

  .option
    padding 2.5em 1.5em 0.5em 1.5em
    text-align left

    .name
      font-weight bold
      margin-right 1em

    .type
      color green

    .default
      color red

    .help
      margin 1em 0 0 1em
      white-space pre-wrap

    .constraint
      margin 1em 0 0 1em

  .options
    td
      text-align left

  .option-category-name
    font-size 20pt
    margin 3em 0 0 0
</style>
