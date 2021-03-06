#!/usr/bin/env python
import os, string, sys
from tools.gpick import *

env = GpickEnvironment(ENV = os.environ, BUILDERS = {'WriteNsisVersion': Builder(action = WriteNsisVersion, suffix = ".nsi")})

vars = Variables(os.path.join(env.GetLaunchDir(), 'user-config.py'))
vars.Add('DESTDIR', 'Directory to install under', '/usr/local')
vars.Add('LOCALEDIR', 'Path to locale directory', '')
vars.Add('DEBARCH', 'Debian package architecture', 'i386')
vars.Add(BoolVariable('ENABLE_NLS', 'Compile with gettext support', True))
vars.Add(BoolVariable('DEBUG', 'Compile with debug information', False))
vars.Add('BUILD_TARGET', 'Build target', '')
vars.Add('TOOLCHAIN', 'Toolchain', 'gcc')
vars.Add(BoolVariable('EXPERIMENTAL_CSS_PARSER', 'Compile with experimental CSS parser', False))
vars.Add('MSVS_VERSION', 'Visual Studio version', '11.0')
vars.Add(BoolVariable('PREBUILD_GRAMMAR', 'Use prebuild grammar files', False))
vars.Add(BoolVariable('USE_GTK3', 'Use GTK3 instead of GTK2', False))
vars.Update(env)

if env['LOCALEDIR'] == '':
	env['LOCALEDIR'] = env['DESTDIR'] + '/share/locale'

v = Variables(os.path.join(env.GetLaunchDir(), 'version.py'))
v.Add('GPICK_BUILD_VERSION', '', '0.0')
v.Update(env)

if not env['BUILD_TARGET']:
	env['BUILD_TARGET'] = sys.platform

if env['BUILD_TARGET'] == 'win32':
	if env['TOOLCHAIN'] == 'msvc':
		env['TARGET_ARCH'] = 'x86'
		env['MSVS'] = {'VERSION': env['MSVS_VERSION']}
		env['MSVC_VERSION'] = env['MSVS_VERSION']
		env["MSVC_SETUP_RUN"] = False
		Tool('msvc')(env)
	else:
		if sys.platform != 'win32':
			env.Tool('crossmingw', toolpath = ['tools'])
		else:
			env.Tool('mingw')

env.AddCustomBuilders()
env.GetVersionInfo()

try:
	umask = os.umask(022)
except OSError: # ignore on systems that don't support umask
	pass

if os.environ.has_key('CC'):
	env['CC'] = os.environ['CC']
if os.environ.has_key('CFLAGS'):
	env['CCFLAGS'] += SCons.Util.CLVar(os.environ['CFLAGS'])
if os.environ.has_key('CXX'):
	env['CXX'] = os.environ['CXX']
if os.environ.has_key('CXXFLAGS'):
	env['CXXFLAGS'] += SCons.Util.CLVar(os.environ['CXXFLAGS'])
if os.environ.has_key('LDFLAGS'):
	env['LINKFLAGS'] += SCons.Util.CLVar(os.environ['LDFLAGS'])

if not env.GetOption('clean'):
	conf = Configure(env)

	programs = {}
	if env['ENABLE_NLS']:
		programs['GETTEXT'] = {'checks':{'msgfmt': 'GETTEXT'}}
		programs['XGETTEXT'] = {'checks':{'xgettext': 'XGETTEXT'}, 'required': False}
		programs['MSGMERGE'] = {'checks':{'msgmerge': 'MSGMERGE'}, 'required': False}
		programs['MSGCAT'] = {'checks':{'msgcat': 'MSGCAT'}, 'required': False}
	programs['RAGEL'] = {'checks':{'ragel': 'RAGEL'}}
	if env['EXPERIMENTAL_CSS_PARSER'] and not env['PREBUILD_GRAMMAR']:
		programs['LEMON'] = {'checks':{'lemon': 'LEMON'}}
		programs['FLEX'] = {'checks':{'flex': 'FLEX'}}
	env.ConfirmPrograms(conf, programs)

	libs = {}

	if not env['TOOLCHAIN'] == 'msvc':
		if not env['USE_GTK3']:
			libs['GTK_PC'] = {'checks':{'gtk+-2.0': '>= 2.24.0'}}
			libs['GIO_PC'] = {'checks':{'gio-unix-2.0': '>= 2.26.0', 'gio-2.0': '>= 2.26.0'}}
		else:
			libs['GTK_PC'] = {'checks':{'gtk+-3.0': '>= 3.0.0'}}
		libs['LUA_PC'] = {'checks':{'lua5.3': '>= 5.3', 'lua': '>= 5.2', 'lua5.2': '>= 5.2'}}
	env.ConfirmLibs(conf, libs)
	env.ConfirmBoost(conf, '1.58')
	env = conf.Finish()

Decider('MD5-timestamp')

if not env['TOOLCHAIN'] == 'msvc':
	if not (os.environ.has_key('CFLAGS') or os.environ.has_key('CXXFLAGS') or os.environ.has_key('LDFLAGS')):
		if env['DEBUG']:
			env.Append(
				CXXFLAGS = ['-Wall', '-g3', '-O0'],
				CFLAGS = ['-Wall', '-g3', '-O0'],
				LINKFLAGS = ['-Wl,-as-needed'],
				)
		else:
			env.Append(
				CPPDEFINES = ['NDEBUG'],
				CDEFINES = ['NDEBUG'],
				CXXFLAGS = ['-Wall', '-O3'],
				CFLAGS = ['-Wall', '-O3'],
				LINKFLAGS = ['-Wl,-as-needed', '-s'],
				)
		env.Append(
			CXXFLAGS = ['-std=c++14'],
		)
	else:
		stdMissing = True
		for flag in env['CXXFLAGS']:
			if flag.startswith('-std='):
				stdMissing = False
				break
		if stdMissing:
			env.Append(
				CXXFLAGS = ['-std=c++14'],
			)

	if env['BUILD_TARGET'] == 'win32':
		env.Append(
				LINKFLAGS = ['-Wl,--enable-auto-import', '-static-libgcc', '-static-libstdc++'],
				CPPDEFINES = ['_WIN32_WINNT=0x0501'],
				)
else:
	env['LINKCOM'] = [env['LINKCOM'], 'mt.exe -nologo -manifest ${TARGET}.manifest -outputresource:$TARGET;1']
	if env['DEBUG']:
		env.Append(
				CXXFLAGS = ['/Od', '/EHsc', '/MD', '/Gy', '/Zi', '/TP', '/wd4819'],
				CPPDEFINES = ['WIN32', '_DEBUG', 'NOMINMAX'],
				LINKFLAGS = ['/MANIFEST', '/DEBUG'],
			)
	else:
		env.Append(
				CXXFLAGS = ['/O2', '/Oi', '/GL', '/EHsc', '/MD', '/Gy', '/Zi', '/TP', '/wd4819'],
				CPPDEFINES = ['WIN32', 'NDEBUG', 'NOMINMAX'],
				LINKFLAGS = ['/MANIFEST', '/LTCG'],
			)

env.Append(
	CPPPATH = ['#extern'],
)

extern_libs = SConscript(['extern/SConscript'], exports = 'env')
executable, tests, parser_files = SConscript(['source/SConscript'], exports = 'env')

env.Alias(target = "build", source=[
	executable,
])

env.Alias(target = "test", source=[
	tests,
])

if 'debian' in COMMAND_LINE_TARGETS:
	SConscript("deb/SConscript", exports = 'env')

if env['ENABLE_NLS']:
	translations = env.Glob('share/locale/*/LC_MESSAGES/gpick.po')
	locales = env.Gettext(translations)
	Depends(executable, locales)
	stripped_locales = []
	for translation in translations:
		stripped_locales.append(env.Msgcat(translation, File(translation).srcnode(), MSGCAT_FLAGS = ['--no-location', '--sort-output', '--no-wrap', '--to-code=utf-8']))
	env.Alias(target = "strip_locales", source = stripped_locales)
	env.Alias(target = "locales", source = locales)
	template_c = env.Xgettext("template_c.pot", env.Glob('source/*.cpp') + env.Glob('source/tools/*.cpp') + env.Glob('source/transformation/*.cpp'), XGETTEXT_FLAGS = ['--keyword=N_', '--from-code=UTF-8', '--package-version="$GPICK_BUILD_VERSION"'])
	template_lua = env.Xgettext("template_lua.pot", env.Glob('share/gpick/*.lua'), XGETTEXT_FLAGS = ['--language=C++', '--keyword=N_', '--from-code=UTF-8', '--package-version="$GPICK_BUILD_VERSION"'])
	template = env.Msgcat("template.pot", [template_c, template_lua], MSGCAT_FLAGS = ['--use-first'])
	env.Alias(target = "template", source = [
		template
	])

env.Alias(target = "install", source = [
	env.InstallProgram(dir = env['DESTDIR'] +'/bin', source = [executable]),
	env.InstallData(dir = env['DESTDIR'] +'/share/metainfo', source = ['share/metainfo/gpick.appdata.xml']),
	env.InstallData(dir = env['DESTDIR'] +'/share/applications', source = ['share/applications/gpick.desktop']),
	env.InstallData(dir = env['DESTDIR'] +'/share/mime/packages', source = ['share/mime/packages/gpick.xml']),
	env.InstallData(dir = env['DESTDIR'] +'/share/doc/gpick', source = ['share/doc/gpick/copyright']),
	env.InstallData(dir = env['DESTDIR'] +'/share/gpick', source = [env.Glob('share/gpick/*.png'), env.Glob('share/gpick/*.lua'), env.Glob('share/gpick/*.txt')]),
	env.InstallData(dir = env['DESTDIR'] +'/share/man/man1', source = ['share/man/man1/gpick.1']),
	env.InstallData(dir = env['DESTDIR'] +'/share/icons/hicolor/48x48/apps/', source = [env.Glob('share/icons/hicolor/48x48/apps/*.png')]),
	env.InstallData(dir = env['DESTDIR'] +'/share/icons/hicolor/scalable/apps/', source = [env.Glob('share/icons/hicolor/scalable/apps/*.svg')]),
	env.InstallDataAutoDir(dir = env['DESTDIR'] + '/share/locale/', relative_dir = 'share/locale/', source = [env.Glob('share/locale/*/LC_MESSAGES/gpick.mo')]),
])

env.Alias(target = "nsis", source = [
	env.WriteNsisVersion("version.py")
])

env.Alias(target = "version", source = [
	env.Template(target = "#version.txt", source = "version.template"),
])

tarFiles = env.GetSourceFiles("(" + RegexEscape(os.sep) + r"\.)|(" + RegexEscape(os.sep) + r"\.svn$)|(^" + RegexEscape(os.sep) + r"build$)", r"(^\.)|(\.pyc$)|(\.orig$)|(~$)|(\.log$)|(\.diff)|(\.mo$)|(\.patch)|(^gpick-.*\.tar\.gz$)|(^user-config\.py$)")

for item in parser_files:
	tarFiles.append(str(item))

if 'TAR' in env:
	env.Alias(target = "tar", source = [
		'version',
		env.Append(TARFLAGS = ['-z']),
		env.Prepend(TARFLAGS = ['--transform', '"s,(^(build/)?),gpick_' + str(env['GPICK_BUILD_VERSION']) + '/,x"']),
		env.Tar('gpick_' + str(env['GPICK_BUILD_VERSION']) + '.tar.gz', tarFiles)
	])

env.Default(executable)

