# Setup
import os
import json
env = Environment(ENV = os.environ)
try:
  env.Tool('config', toolpath = [os.environ.get('CBANG_HOME')])
except Exception as e:
  raise Exception('CBANG_HOME not set?\n' + str(e))

env.CBLoadTools('compiler cbang dist build_info packager resources')

# Override variables
env.CBAddVariables(
  BoolVariable('mostly_static', 'Link most libraries statically', 1))

conf = env.CBConfigure()

# Settings
name    = 'fah-node'
version = json.load(open('package.json', 'r'))['version']

# Config vars
author = 'Joseph Coffland <joseph@cauldrondevelopment.com>'
env.Replace(PACKAGE_VERSION   = version)
env.Replace(PACKAGE_AUTHOR    = author)
env.Replace(PACKAGE_ORG       = 'foldingathome.org')
env.Replace(PACKAGE_COPYRIGHT = 'foldingathome.org, 2023')
env.Replace(PACKAGE_HOMEPAGE  = 'https://foldingathome.org/')
env.Replace(PACKAGE_LICENSE   = 'GPL 3+')
env.Replace(RESOURCES_NS      = 'FAH::Node')
env.Replace(BUILD_INFO_NS     = 'FAH::BuildInfo')

if not env.GetOption('clean') and not 'package' in COMMAND_LINE_TARGETS:
  conf.CBConfig('compiler')
  conf.CBConfig('cbang')
  env.CBDefine('USING_CBANG') # Using CBANG macro namespace

  if not env.CBConfigEnabled('event'):
    raise Exception('libevent support in C! is required')

  if not env.CBConfigEnabled('openssl'):
    raise Exception('OpenSSL support in C! is required')

  env.Append(PREFER_DYNAMIC = 'rt tcmalloc Xss m'.split())

conf.Finish()

# Resources
if not 'package' in COMMAND_LINE_TARGETS:
  from subprocess import check_call
  check_call(['npm', 'install'])
  check_call(['npm', 'run', 'build'])

# Main
Export('env name')
prog = SConscript('src/%s.scons' % name, variant_dir = 'build', duplicate = 0)
Default(prog)
Clean(prog, ['build', 'config.log'])

# Dist
docs = ['README.md', 'CHANGELOG.md']
tar  = env.TarBZ2Dist(name, docs + [name, 'scripts'])
Alias('dist', tar)
AlwaysBuild(tar)

# TODO Complete descriptions
description = '''\
Folding@home is a distributed computing project using volunteered computer
resources.

fah-node can be used to authenticate, monitor and control your swarm of
Folding@home clients remotely.
'''

short_description = description


# Package
if 'package' in COMMAND_LINE_TARGETS:
  pkg = env.Packager(
    name,
    version = version,
    maintainer = author,
    vendor = 'foldingathome.org',
    url = 'https://foldingathome.org/',
    license = 'copyright',
    bug_url = 'https://github.com/FoldingAtHome/fah-node/issues',
    summary = 'Folding@home Node',
    description = description,
    prefix = '/usr',

    documents = docs,
    programs = [str(prog[0])],
    scripts = ['scripts/fah-node-ports'],
    changelog = 'changelog',
    platform_independent = ['share/sample-config.xml',
                'share/fah-node.service'],

    deb_directory = 'debian',
    deb_section = 'science',
    deb_depends = 'debconf | debconf-2.0, libc6, bzip2, zlib1g, systemd',
    deb_pre_depends = 'adduser, ssl-cert, curl',
    deb_priority = 'optional',
    )

  AlwaysBuild(pkg)
  env.Alias('package', pkg)
