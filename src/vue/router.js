/******************************************************************************\

                      Copyright 2017-2021. foldingathome.org
                    Copyright 2001-2017. Stanford University.
                               All Rights Reserved.

                  For information regarding this software email:
                                 Joseph Coffland
                          joseph@cauldrondevelopment.com

\******************************************************************************/

import {createRouter, createWebHistory} from 'vue-router'

import MainPage        from './MainPage.vue'
import ConnectionsPage from './ConnectionsPage.vue'
import HelpPage        from './HelpPage.vue'
import MessagesPage    from './MessagesPage.vue'
import NotFoundPage    from './NotFoundPage.vue'
import ServerPage      from './ServerPage.vue'


export default createRouter({
  history: createWebHistory(),
  base: '/admin/',
  routes: [
    {path: '/admin/',            redirect: '/admin/main'},
    {path: '/admin/main',        component: MainPage},
    {path: '/admin/connections', component: ConnectionsPage},
    {path: '/admin/help',        component: HelpPage},
    {path: '/admin/messages',    component: MessagesPage},
    {path: '/admin/server',      component: ServerPage},
    {path: '/:pathMatch(.*)',    component: NotFoundPage},
  ]
})
