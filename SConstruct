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

# Package info
with open('package.json', 'r') as f:
  package_info = json.load(f)

name              = package_info['name']
version           = package_info['version']
description       = package_info['description']
short_description = description

# Config vars
env.Replace(PACKAGE_VERSION   = version)
env.Replace(PACKAGE_AUTHOR    = package_info['author'])
env.Replace(PACKAGE_ORG       = package_info['org'])
env.Replace(PACKAGE_COPYRIGHT = package_info["copyright"])
env.Replace(PACKAGE_HOMEPAGE  = package_info["homepage"])
env.Replace(PACKAGE_LICENSE   = package_info["license"])
env.Replace(RESOURCES_NS      = 'FAH::Node')
env.Replace(BUILD_INFO_NS     = 'FAH::Node::BuildInfo')

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
if not ('package' in COMMAND_LINE_TARGETS or 'dist' in COMMAND_LINE_TARGETS):
  from subprocess import check_call
  check_call(['npm', 'install'])
  check_call(['npm', 'run', 'build'])

# Main
Export('env name')
prog = SConscript('src/%s.scons' % name, variant_dir = 'build', duplicate = 0)
Default(prog)
Clean(prog, ['build', 'config.log'])

# Dist
docs = ['README.md', 'CHANGELOG.md', 'LICENSE']
tar  = env.TarBZ2Dist(name, docs + [name, 'scripts'])
Alias('dist', tar)
AlwaysBuild(tar)

# Package
if 'package' in COMMAND_LINE_TARGETS:
  pkg = env.Packager(
    name,
    version = version,
    maintainer = package_info['author'],
    vendor = package_info['org'],
    url = package_info['homepage'],
    license = 'LICENSE',
    bug_url = package_info['bugs']['url'],
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
