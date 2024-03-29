/******************************************************************************\

                      Copyright 2017-2021. foldingathome.org
                    Copyright 2001-2017. Stanford University.
                               All Rights Reserved.

                  For information regarding this software email:
                                 Joseph Coffland
                          joseph@cauldrondevelopment.com

\******************************************************************************/

import {createApp}    from 'vue'
import filters        from './filters'
import util           from './util'
import API            from './api'
import router         from './router'
import App            from './App.vue'
import RateStats      from './RateStats.vue'
import CountStats     from './CountStats.vue'

let app = createApp(App)

app.config.globalProperties.$filters = filters
app.config.globalProperties.$util    = util
app.config.globalProperties.$api     = new API()

app.component('RateStats',      RateStats)
app.component('CountStats',     CountStats)

app.use(router)

app.mount('#app')
