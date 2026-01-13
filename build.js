/******************************************************************************\

                   This file is part of FoldingAtHome/fah-node

        The fah-node enables secure remote access to Folding@home clients.
                    Copyright (c) 2023-2026, foldingathome.org
                               All rights reserved.

       This program is free software; you can redistribute it and/or modify
       it under the terms of the GNU General Public License as published by
        the Free Software Foundation; either version 3 of the License, or
                       (at your option) any later version.

         This program is distributed in the hope that it will be useful,
          but WITHOUT ANY WARRANTY; without even the implied warranty of
          MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
                   GNU General Public License for more details.

     You should have received a copy of the GNU General Public License along
     with this program; if not, write to the Free Software Foundation, Inc.,
           51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

                  For information regarding this software email:
                                 Joseph Coffland
                          joseph@cauldrondevelopment.com

\******************************************************************************/

const esbuild   = require('esbuild')
const pluginVue = require('esbuild-plugin-vue3')
const fs        = require('fs-extra')
const path      = require('path')
const pug       = require('pug')
const chokidar  = require('chokidar')
const pkg       = require('./package.json')

const outdir = 'build/res'
const watchPaths = [
  'src/index.pug',
  'src/admin.pug',
  'src/login.pug',
  'src/vue'
]


// Parse args
const args = process.argv.slice(2)
let watch = false
for (let arg of args)
  if (arg == '--watch') watch = true


async function build() {
  console.log('Building...')

  await fs.remove(outdir)
  await fs.copy('src/resources', outdir)

  let result = await esbuild.build({
    entryPoints: ['src/vue/main.js'],
    bundle: true,
    minify: true,
    sourcemap: true,
    outdir: outdir + '/http',
    entryNames: '[name]-[hash]',
    metafile: true,
    plugins: [pluginVue()]
  })

  let vars    = {version: pkg.version}
  let outputs = Object.keys(result.metafile.outputs)

  for (let output of outputs) {
    let href = '/' + path.basename(output)
    if (output.endsWith('.js'))  vars.js  = href
    if (output.endsWith('.css')) vars.css = href
  }

  for (let name of ['index', 'admin', 'login']) {
    let html = pug.compileFile('src/' + name + '.pug')(vars)
    await fs.writeFile(outdir + '/http/' + name + '.html', html)
  }
}


async function main() {
  await build()

  if (watch) {
    console.log('Watching...')
    chokidar.watch(watchPaths)
      .on('change', async (path, stats) => {
        console.log('Change detected in', path)
        await build()
      })
  }
}


main().catch(e => console.log(e))
