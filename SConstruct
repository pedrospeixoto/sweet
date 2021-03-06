#! /usr/bin/env python

import os

import commands
import re
import sys
import platform
import subprocess

from python_mods import CompileXMLOptions

hostname=subprocess.check_output('hostname')
hostname = hostname.replace("\n", '')
hostname = hostname.replace("\r", '')

env = Environment()


###################################################################
# fix environment vars (not imported by default)
###################################################################

# import all environment variables (expecially PATH, LD_LIBRARY_PATH, LD_PATH)
env.Append(ENV=os.environ)



files = os.listdir('src/programs/')
files = sorted(files)
example_programs = []
for f in files:
	example_programs.append(f[0:-4])
env['example_programs'] = example_programs


files = os.listdir('src/unit_tests/')
files = sorted(files)
unit_tests_programs = []
for f in files:
	unit_tests_programs.append(f[0:-4])
env['unit_tests_programs'] = unit_tests_programs



################################################################
# XML CONFIGURATION
################################################################


AddOption(	'--xml-config',		dest='xml_config',	type='string',	default='',	help='xml configuration file with compile options')
env['xml_config'] = GetOption('xml_config')

if env['xml_config'] != '':
	env = CompileXMLOptions.load(env['xml_config'], env)



################################################################
# Compile options
################################################################


AddOption(      '--mode',
		dest='mode',
		type='choice',
		choices=['debug', 'release'],
		default='release',
		help='specify compiler to use: debug, release [default: %default]'
)
env['mode'] = GetOption('mode')


AddOption(	'--compiler',
		dest='compiler',
		type='choice',
		choices=['gnu', 'intel', 'llvm', 'pgi'],
		default='gnu',
		help='specify compiler to use: gnu, intel, llvm, pgi [default: %default]'
)
env['compiler'] = GetOption('compiler')


AddOption(	'--numa-block-allocator',
		dest='numa_block_allocator',
		type='choice',
		choices=['0', '1', '2'],
		default='0',
		help='Specify allocation method to use: 0: default system\'s malloc, 1: allocation with NUMA granularity, 2: allocation with thread granularity [default: %default]'
)
env['numa_block_allocator'] = GetOption('numa_block_allocator')


AddOption(	'--gxx-toolchain',
		dest='gxx-toolchain',
		type='string',
		default='',
		help='specify gcc toolchain for intel and llvm compiler, e.g. g++-4.6, default: deactivated'
)
env['gxx_toolchain'] = GetOption('gxx-toolchain')


AddOption(	'--simd',
		dest='simd',
		type='choice',
		choices=['enable', 'disable'],
		default='enable',
		help="Use SIMD for operations such as folding [default: %default]"
)
env['simd'] = GetOption('simd')

env.Append(CXXFLAGS=' -DSWEET_SIMD_ENABLE='+('1' if env['simd']=='enable' else '0'))


AddOption(	'--debug-symbols',
		dest='debug_symbols',
		type='choice',
		choices=['enable', 'disable'],
		default='enable',
		help="Create binary with debug symbols [default: %default]"
)
env['debug_symbols'] = GetOption('debug_symbols')



AddOption(	'--spectral-space',
		dest='spectral_space',
		type='choice',
		choices=['enable', 'disable'],
		default='disable',
		help="Use spectral space for operations such as folding [default: %default]"
)
env['spectral_space'] = GetOption('spectral_space')


AddOption(	'--libfft',
		dest='libfft',
		type='choice',
		choices=['enable', 'disable'],
		default='enable',
		help="Enable libFFT [default: %default]"
)
env['libfft'] = GetOption('libfft')


#
# LIB XML
#
AddOption(	'--libxml',
		dest='libxml',
		type='choice',
		choices=['enable','disable'],
		default='disable',
		help='Compile with libXML related code: enable, disable [default: %default]'
)
env['libxml'] = GetOption('libxml')

env.Append(CXXFLAGS=' -DCONFIG_ENABLE_LIBXML='+('1' if env['libxml']=='enable' else '0'))
if env['libxml'] == 'enable':
	env.ParseConfig("xml2-config --cflags --libs")


AddOption(	'--spectral-dealiasing',
		dest='spectral_dealiasing',
		type='choice',
		choices=['enable','disable'],
		default='disable',
		help='spectral dealiasing (3N/2-1 rule): enable, disable [default: %default]'
)
env['spectral_dealiasing'] = GetOption('spectral_dealiasing')


AddOption(	'--gui',
		dest='gui',
		type='choice',
		choices=['enable','disable'],
		default='disable',
		help='gui: enable, disable [default: %default]'
)
env['gui'] = GetOption('gui')




AddOption(	'--rexi-parallel-sum',
		dest='rexi_thread_parallel_sum',
		type='choice',
		choices=['enable','disable'],
		default='disable',
		help='Use a par for loop over the sum in REXI: enable, disable [default: %default]'
)
env['rexi_thread_parallel_sum'] = GetOption('rexi_thread_parallel_sum')


AddOption(	'--sweet-mpi',
		dest='sweet_mpi',
		type='choice',
		choices=['enable','disable'],
		default='disable',
		help='Enable MPI commands e.g. for REXI sum: enable, disable [default: %default]'
)
env['sweet_mpi'] = GetOption('sweet_mpi')



AddOption(      '--program',
		dest='program',
		type='choice',
		choices=example_programs,
		default='',
		help='Specify program to compile: '+', '.join(example_programs)+' '*80+' [default: %default]'
)
env['compile_program'] = GetOption('program')


AddOption(      '--program-binary-name',
		dest='program_binary_name',
		type='string',
		action='store',
		help='Name of program binary, default: [auto]',
		default=''
)
env['program_binary_name'] = GetOption('program_binary_name')


AddOption(      '--unit-test',
		dest='unit_test',
		type='choice',
		choices=unit_tests_programs,
		default='',
		help='Specify unit tests to compile: '+', '.join(unit_tests_programs)+' '*80+' [default: %default]'
)
env['unit_test'] = GetOption('unit_test')


threading_constraints = ['off', 'omp']
AddOption(	'--threading',
		dest='threading',
		type='string',	
		default='off',
		help='Threading to use '+' / '.join(threading_constraints)+', default: off'
)
env['threading'] = GetOption('threading')


AddOption(	'--cxx-flags',
		dest='cxx_flags',
		type='string',	
		default='',
		help='Additional cxx-flags, default: ""'
)

env['cxx_flags'] = GetOption('cxx_flags')
env.Append(CXXFLAGS = env['cxx_flags'])


AddOption(	'--ld-flags',
		dest='ld_flags',
		type='string',	
		default='',
		help='Additional ld-flags, default: ""'
)
env['ld_flags'] = GetOption('ld_flags')

env['ld_flags'] = GetOption('ld_flags')
env.Append(LINKFLAGS = env['ld_flags'])


env['fortran_source'] = 'disable'


###########################################
# Compile options
###########################################

# set default to first example program
if env['compile_program'] != '':
	env['program_name'] = env['compile_program']
elif env['unit_test'] != '':
	env['program_name'] = env['unit_test']
else:
	env['program_name'] = 'DUMMY'
	print("\n")
	print("Neither a program name, nor a unit test is given:\n")
	print("  use --program=[program name] to specify the program\n")
	print("  or --unit-test=[unit test] to specify a unit test\n")



env.Append(CXXFLAGS = ' -DSWEET_PROGRAM_NAME='+env['program_name'])



exec_name=env['program_name']


if env['spectral_space'] == 'enable':
	if env['libfft'] != 'enable':
		print("Option --libfft is disabled!")
		Exit(1)
	env.Append(CXXFLAGS = ' -DSWEET_USE_SPECTRAL_SPACE=1')
	exec_name+='_spectral'
else:
	env.Append(CXXFLAGS = ' -DSWEET_USE_SPECTRAL_SPACE=0')

if env['libfft'] == 'enable':
	env.Append(CXXFLAGS = ' -DSWEET_USE_LIBFFT=1')
	exec_name+='_libfft'
else:
	env.Append(CXXFLAGS = ' -DSWEET_USE_LIBFFT=0')



if env['sweet_mpi'] == 'enable':
	env.Append(CXXFLAGS = ' -DSWEET_MPI=1')
else:
	env.Append(CXXFLAGS = ' -DSWEET_MPI=0')



if env['spectral_dealiasing'] == 'enable':
	env.Append(CXXFLAGS = ' -DSWEET_USE_SPECTRAL_DEALIASING=1')
	exec_name+='_dealiasing'
else:
	env.Append(CXXFLAGS = ' -DSWEET_USE_SPECTRAL_DEALIASING=0')
	
	

if env['gui']=='enable':
	exec_name+='_gui'

#if env['program_binary_name_override'] != '':
#	exec_name = env['program_binary_name_override']



if env['compiler'] == 'gnu':
	reqversion = [4,6,1]

	#
	# get gcc version using -v instead of -dumpversion since SUSE gnu compiler
	# returns only 2 instead of 3 digits with -dumpversion
	#
	gccv = commands.getoutput('g++ -v').splitlines()

	# updated to search for 'gcc version ' line prefix
	found = False
	found_line = ''

	search_string = 'gcc version '
	for l in gccv:
		if l[:len(search_string)] == search_string:
			found_line = l
			found = True

			gccversion = found_line.split(' ')[2].split('.')
			break

	if not found:
		search_string = 'gcc-Version '
		for l in gccv:
			if l[:len(search_string)] == search_string:
				found_line = l
				found = True
	
				gccversion = found_line.split(' ')[1].split('.')
				break

	if not found:
		print search_string+" not found"
		sys.exit(-1)

	for i in range(0, 3):
		if (int(gccversion[i]) > int(reqversion[i])):
			break
		if (int(gccversion[i]) < int(reqversion[i])):
			print 'At least GCC Version 4.6.1 necessary.'
			Exit(1)

	# eclipse specific flag
	env.Append(CXXFLAGS=' -fmessage-length=0')

	# c++0x flag
	env.Append(CXXFLAGS=' -std=c++0x')

	# be pedantic to avoid stupid programming errors
#	env.Append(CXXFLAGS=' -pedantic')

	# speedup compilation - remove this when compiler slows down or segfaults by running out of memory
	env.Append(CXXFLAGS=' -pipe')

	# activate gnu C++ compiler

	if env['fortran_source']=='enable':
		env.Replace(FORTRAN='gfortran')
		env.Replace(F90='gfortran')
		env.Append(FORTRANFLAGS=' -cpp')
		env.Append(F90FLAGS=' -cpp')
		env.Append(LIBS=['gfortran'])

#	env.Replace(CXX = 'g++-4.7')
	env.Replace(CXX = 'g++')


elif env['compiler'] == 'intel':
	reqversion = [12,1]
	iccversion_line = commands.getoutput('icpc -dumpversion')

	if iccversion_line != 'Mainline':
		iccversion = iccversion_line.split('.')
		for i in range(0, 2):
			if (int(iccversion[i]) > int(reqversion[i])):
				break
			if (int(iccversion[i]) < int(reqversion[i])):
				print 'ICPC Version 12.1 necessary.'
				Exit(1)

	# override g++ called by intel compiler to determine location of header files
	if env['gxx_toolchain'] != '':
		env.Append(CXXFLAGS='-gxx-name='+env['gxx_toolchain'])
		env.Append(LINKFLAGS='-gxx-name='+env['gxx_toolchain'])

	env.Append(LINKFLAGS=' -shared-intel')
	env.Append(LINKFLAGS=' -shared-libgcc')
	env.Append(LINKFLAGS=' -debug inline-debug-info')

	# eclipse specific flag
	env.Append(CXXFLAGS=' -fmessage-length=0')

	# c++0x flag
	env.Append(CXXFLAGS=' -std=c++0x')

	# output more warnings
	env.Append(CXXFLAGS=' -w1')


	# compiler option which has to be appended for icpc 12.1 without update1
#	env.Append(CXXFLAGS=' -U__GXX_EXPERIMENTAL_CXX0X__')


	# UBUNTU FIX for i386 systems
	lines = commands.getoutput('uname -i').splitlines()

	for i in lines:
		if i == 'i386':
			env.Append(CXXFLAGS=' -I/usr/include/i386-linux-gnu/')

	# SSE 4.2
	env.Replace(CXX = 'icpc')
	env.Replace(LINK='icpc')

	if env['fortran_source'] == 'enable':
		env.Append(LIBS=['ifcore'])
		env.Replace(FORTRAN='ifort')
		env.Replace(F90='ifort')
		env.Append(FORTRANFLAGS=' -fpp')
		env.Append(F90FLAGS=' -fpp')


elif env['compiler'] == 'pgi':
	# activate pgi
	env.Replace(CXX = 'pgc++')

	# c++0x flag
	env.Append(CXXFLAGS=' --c++11')

	# last gcc compatible version: 4.6
#	gnu_compat_version = commands.getoutput("`g++-4.6 -v 2>&1 | grep ' version ' | cut -d' ' -f3")

	# set gnu version
#	env.Append(CXXFLAGS=' --gnu_version '+gnu_compat_version)
#	env.Append(CXXFLAGS=' --gnu_version 4.8.0')
	env.Append(CXXFLAGS=' -D__GXX_EXPERIMENTAL_CXX0X__ --gnu_extensions')

	# be pedantic to avoid stupid programming errors
#	env.Append(CXXFLAGS=' -pedantic')

	# speedup compilation - remove this when compiler slows down or segfaults by running out of memory
#	env.Append(CXXFLAGS=' -pipe')

	if env['fortran_source'] == 'enable':
		print "TODO: PGI compiler not yet supported with fortran enabled"
		Exit(-1)


elif env['compiler'] == 'llvm':
	reqversion = [3,1]

	if env['gxx_toolchain'] != '':
		env.Append(CXXFLAGS=' --gcc-toolchain='+env['gxx_toolchain'])

	# eclipse specific flag
	env.Append(CXXFLAGS=' -fmessage-length=0')

	# c++0x flag
	env.Append(CXXFLAGS=' -std=c++0x')

	# support __float128
#	env.Append(CXXFLAGS=' -D__STRICT_ANSI__')

	# be pedantic to avoid stupid programming errors
	env.Append(CXXFLAGS=' -pedantic')

	# speedup compilation - remove this when compiler slows down or segfaults by running out of memory
	env.Append(CXXFLAGS=' -pipe')

	# activate gnu C++ compiler

	# todo: fix me also for intel mpicxx compiler
	env.Replace(CXX = 'clang++')

	if env['fortran_source'] == 'enable':
		print "TODO: LLVM compiler not yet supported with fortran enabled"
		Exit(-1)



if env['mode'] == 'debug':
	env.Append(CXXFLAGS=' -DSWEET_DEBUG_MODE=1')

	if env['compiler'] == 'gnu':
		env.Append(CXXFLAGS='-O0 -g3 -Wall')

		# integer overflow check
		env.Append(CXXFLAGS=' -ftrapv')

	elif env['compiler'] == 'llvm':
		env.Append(CXXFLAGS='-O0 -g3 -Wall')

	elif env['compiler'] == 'intel':
		env.Append(CXXFLAGS='-O0 -g -traceback')
#		env.Append(CXXFLAGS=' -fp-trap=common')

	elif env['compiler'] == 'pgi':
		env.Append(CXXFLAGS='-O0 -g -traceback')


	if env['fortran_source'] == 'enable':
		if env['compiler'] == 'gnu':
			env.Append(FORTRANFLAGS='-g -O0')
			env.Append(F90FLAGS='-g -O0')
		elif env['compiler'] == 'intel':
			env.Append(FORTRANFLAGS='-g -O0 -traceback')
			env.Append(F90FLAGS='-g -O0 -traceback')


elif env['mode'] == 'release':
	env.Append(CXXFLAGS=' -DSWEET_DEBUG_MODE=0')

	# deactivate assertion calls
	env.Append(CXXFLAGS=' -DNDEBUG=1')

	if env['compiler'] == 'gnu':
		env.Append(CXXFLAGS=' -O3 -mtune=native')

	elif env['compiler'] == 'llvm':
		env.Append(CXXFLAGS=' -O3 -mtune=native')

	elif env['compiler'] == 'intel':
		env.Append(CXXFLAGS=' -O2 -fno-alias')
		env.Append(CXXFLAGS=' -xHost')
#		env.Append(CXXFLAGS=' -ipo')
#		env.Append(CXXFLAGS=' -fast')

	elif env['compiler'] == 'pgi':
		env.Append(CXXFLAGS='-O3 -fast -Mipa=fast,inline -Msmartalloc')

	if env['fortran_source'] == 'enable':
		if env['compiler'] == 'gnu':
			env.Append(FORTRANFLAGS=' -O2')
			env.Append(F90FLAGS=' -O2')
		elif env['compiler'] == 'intel':
			env.Append(FORTRANFLAGS=' -O2')
			env.Append(F90FLAGS=' -O2')
#			env.Append(FORTRANFLAGS=' -ipo')
#			env.Append(F90FLAGS=' -ipo')
#			env.Append(FORTRANFLAGS='-fast')
#			env.Append(F90FLAGS=' -fast')




if env['gui'] == 'enable':
	# compile flags
	env.Append(CXXFLAGS=' -I'+os.environ['HOME']+'/local/include')
	env.Append(CXXFLAGS=' -DSWEET_GUI=1')

	# linker flags

	# add nvidia lib path when running on atsccs* workstation
#	hostname = commands.getoutput('uname -n')
#	if re.match("atsccs.*", hostname) or  re.match("laptop.*", hostname):
#		env.Append(LIBPATH=['/usr/lib/nvidia-current/'])

#	env.Append(LIBPATH=[os.environ['HOME']+'/local/lib'])
	env.Append(LIBS=['GL'])

	reqversion = [2,0,0]
	sdlversion = commands.getoutput('sdl2-config --version').split('.')

	for i in range(0, 3):
		if (int(sdlversion[i]) > int(reqversion[i])):
			break;
		if (int(sdlversion[i]) < int(reqversion[i])):
			print 'libSDL Version 2.0.0 necessary.'
			Exit(1)

	env.ParseConfig("sdl2-config --cflags --libs")
#	env.ParseConfig("pkg-config SDL2_image --cflags --libs")
	env.ParseConfig("pkg-config freetype2 --cflags --libs")
else:
	env.Append(CXXFLAGS=' -DSWEET_GUI=0')

  


if env['sweet_mpi'] == 'enable':
	print "Enabling MPI for REXI"
	print "Warning: Compiler checks not done"

	if env['compiler'] == 'gnu':
		env.Replace(CXX = 'mpiCC')
		env.Replace(LINK = 'mpiCC')

	elif env['compiler'] == 'intel':
		env.Replace(CXX = 'mpiicpc')
		env.Replace(LINK = 'mpiicpc')

	if env['threading'] != 'off' and env['compiler'] == 'intel':
		env.Append(CXXFLAGS=' -mt_mpi')
		env.Append(LINKFLAGS=' -mt_mpi')




if env['threading'] in ['omp']:
	exec_name+='_'+env['threading']

if env['rexi_thread_parallel_sum']=='enable':
	exec_name+='_rexipar'

env.Append(CXXFLAGS=' -DNUMA_BLOCK_ALLOCATOR_TYPE='+env['numa_block_allocator'])

if env['numa_block_allocator'] != '0':
	exec_name+='_numaallocator'+env['numa_block_allocator']
	env.Append(LIBS=['numa'])


exec_name += '_'+env['compiler']
exec_name += '_'+env['mode']

if env['threading'] == 'omp':
	env.Append(CXXFLAGS=' -fopenmp')
	env.Append(LINKFLAGS=' -fopenmp')
	env.Append(CXXFLAGS=' -DSWEET_THREADING=1')
else:
	env.Append(CXXFLAGS=' -DSWEET_THREADING=0')


if env['libfft'] == 'enable':
	env.Append(LIBS=['fftw3'])

	if env['threading'] == 'omp':
		env.Append(LIBS=['fftw3_omp'])

if env['rexi_thread_parallel_sum'] == 'enable' and env['threading'] == 'omp':
	print 'ERROR: "REXI Parallel Sum" and "Threading" is both activated'
	sys.exit(1)

if env['rexi_thread_parallel_sum'] == 'enable' or env['threading'] == 'omp':
	env.Append(LINKFLAGS=' -fopenmp')
	env.Append(CXXFLAGS=' -fopenmp')

if env['rexi_thread_parallel_sum'] == 'enable':
	env.Append(CXXFLAGS=' -DSWEET_REXI_THREAD_PARALLEL_SUM=1')
else:
	env.Append(CXXFLAGS=' -DSWEET_REXI_THREAD_PARALLEL_SUM=0')


if env['program_binary_name'] != '':
	exec_name = env['program_binary_name']

if env['debug_symbols'] == 'enable':
	env.Append(CXXFLAGS = '-g')
	env.Append(LINKFLAGS = '-g')

	if env['compiler'] == 'intel':
		env.Append(CXXFLAGS = ' -O2 -shared-intel -shared-libgcc -debug inline-debug-info')
		env.Append(LINKFLAGS = ' -O2 -shared-intel  -shared-libgcc -debug inline-debug-info')



#
# build directory
#
build_dir='/tmp/scons_build_'+exec_name+'/'

env.Append(CPPPATH = ['/usr/local/include', '/usr/include'])

# MAC CLUSTER
#print hostname
if hostname[0:4] == "mac-":
	env.Append(CPPPATH = ['/usr/local/include', '/usr/include', '/lrz/sys/libraries/fftw/3.3.3/avx/include'])
	env.Append(LINKFLAGS=' -L/lrz/sys/libraries/fftw/3.3.3/avx/lib_omp ')
	env.Append(LINKFLAGS=' -L/home/hpc/pr63so/di69fol/local/numactl-2.0.8/lib64 ')
	env.Append(CPPPATH = ['/home/hpc/pr63so/di69fol/local/numactl-2.0.8/include'])

# also include the 'src' directory to search for dependencies
env.Append(CPPPATH = ['.', './src/', './src/include'])


######################
# get source code files
######################

env.src_files = []

# local software directories
env.Append(LINKFLAGS=['-L./local_software/local/lib'])
env.Append(CPPPATH=['./local_software/local/include'])

if env['program_name'] != 'DUMMY':


	Export('env')
#	SConscript('src/SConscript', variant_dir=build_dir, duplicate=0)
	SConscript('./sconscript', variant_dir=build_dir, duplicate=0)
	Import('env')

	print
	print '            Program: '+env['program_name']
	print 'Building executable: '+exec_name
	print

	env.Program('build/'+exec_name, env.src_files)

