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

  for (let name of ['index', 'admin']) {
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
