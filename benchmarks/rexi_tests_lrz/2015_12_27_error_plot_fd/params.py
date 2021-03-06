#! /usr/bin/python2

import subprocess
import sys
import os
import time
from subprocess import PIPE
import socket

default_params = ''

output_file_prefix = 'output'
if len(sys.argv) > 1:
	output_file_prefix = sys.argv[1]

#
# SCENARIO
#
# 0: radial dam break
# 1: gaussian
# 2: balanced steady state u
# 3: balanced steady state v
# 4: diamond initial condition
# 5: waves
#default_params += ' -s 5'
default_params += ' --initial-freq-x-mul=2.0'
default_params += ' --initial-freq-y-mul=1.0'
scenario_name = "SinCos Waves"


# FD/Spectral time stepping
run_method_0 = True
# rexi
run_method_rexi = False
# rexi par
run_method_rexipar = True


timeout = "02:00:00"


curdir_name = os.getcwd()
print ("Current working directory: "+curdir_name)

#os.chdir('../../../')


#
# run for 1 seconds
#
max_time = 50

#
# time step size for coarse time steps
#
dt = 5.0

#
# order of time step for RK
# Use order 4 to make time errors very small to make the spatial error dominate
#
timestep_order = 4

cfl=0.3

print "Max simulation time: "+str(max_time)
print "Time step size for REXI time step: "+str(dt)
print "Time step order: "+str(timestep_order)

#
# default params
#
default_params += ' -f 1  -g 1 -H 1 -X 1 -Y 1 --compute-error 1 -t '+str(max_time)

# Use higher-order time stepping?
default_params += ' -R '+str(timestep_order)


###########################
# threads
###########################

# 24 threads per node on Hartree center
#T_list = [24]
# Use only single-threaded test (Parallelization-in-time only)
#thread_list = [1, 2, 4, 8, 12, 24]
thread_list = [28]


###########################
# MPI RANKS
###########################
#mpi_ranks = [1, 2, 4, 8, 16, 32, 64, 128, 256]
mpi_ranks = [1, 2, 4, 8, 16, 32, 64, 128, 170, 256, 512, 768, 1024, 1536, 2048, 4096]
mpi_ranks = [1]


# Maximum of total threads (MPI ranks x threads)
max_total_threads = 4*1024


###########################
# resolutions
###########################

res_list = [32, 64, 128, 256, 512]



###########################
# M REXI sampling points
###########################
M_list = []
m = 8
while m < 2000:
	M_list.append(m)
	m *= 2;
#M_list = [64, 128, 256, 512, 1024, 2048, 2048*4, 2048*8, 2048*16, 2048*32, 2048*64]
M_list = [32, 64, 128, 256, 512]
M_list = [8, 16, 32, 64, 128, 256, 512, 1024, 2048, 2048*2, 2048*4, 2048*8, 2048*16]
#M_list = [32, 64, 128, 256, 512, 1024, 1024*4, 1024*16, 1024*32]

###########################
# MEM ALLOC
###########################
A_list = [0, 1, 2]
A_list = [1]


###########################
# HYPER VISCOSITY
###########################
hyperviscosity = {}
# http://math.boisestate.edu/~wright/research/FlyerEtAl2012.pdf
# not necessary for these short-range runs
for n in res_list:
	hyperviscosity[n] = 4.*pow(float(n), float(-4))
	hyperviscosity[n] = 0


comp_spec='scons --compiler=intel --sweet-mpi=enable --program=swe_rexi --spectral-space=enable --libfft=enable --spectral-dealiasing=disable --mode=release'
comp_cart='scons --compiler=intel --sweet-mpi=enable --program=swe_rexi --spectral-space=disable --libfft=enable --spectral-dealiasing=disable --mode=release'

comp_rexi='scons --compiler=intel --sweet-mpi=enable --program=swe_rexi --spectral-space=disable --libfft=enable --rexi-parallel-sum=disable --spectral-dealiasing=disable --mode=release'
comp_rexi_par='scons --compiler=intel --sweet-mpi=enable --program=swe_rexi --spectral-space=disable --libfft=enable --rexi-parallel-sum=enable --spectral-dealiasing=disable --mode=release'


if False:
	tests = []
else:
	# short description, binary, parameters, title
	tests =	[
#		[	'nr_fd_spec_agrid',	' -S 0 --timestepping-mode 0 --staggering 0 -C '+str(cfl), 	'Finite differences in Fourier space, A-grid',		comp_spec, 'nr_fd_spec_agrid'	],
#		[	'nr_fd_cart_agrid',	' -S 0 --timestepping-mode 0 --staggering 0 -C '+str(cfl), 	'Finite differences in Cartesian space, A-grid',	comp_cart, 'nr_fd_cart_agrid'	],

#		[	'nr_fd_spec_cgrid',	' -S 0 --timestepping-mode 0 --staggering 1 -C '+str(cfl), 	'Finite differences in Fourier space, C-grid',		comp_spec, 'nr_fd_spec_cgrid'	],
		[	'nr_fd_cart_cgrid',	' -S 0 --timestepping-mode 0 --staggering 1 -C '+str(cfl), 	'Finite differences in Cartesian space, C-grid',	comp_cart, 'nr_fd_cart_cgrid'	],

#		[	'nr_spec_spec_agrid',	' -S 1 --timestepping-mode 0 --staggering 0 -C '+str(cfl), 	'Spectral derivatives in Fourier space, A-grid',	comp_spec, 'nr_spec_spec_agrid'	],
	]



# add rexi tests
for m in M_list:
#	tests.append(['rexi_m'+str(m).zfill(6),		' -S 1 --use-specdiff-for-complex-array 1 --rexi-h 0.2 --timestepping-mode 1 --staggering 0 --rexi-m='+str(m)+' -C '+str(-dt),	'REXI M='+str(m),		comp_rexi,	'rexi_m'])
#	tests.append(['rexi_par_m'+str(m).zfill(6),	' -S 1 --use-specdiff-for-complex-array 1 --rexi-h 0.2 --timestepping-mode 1 --staggering 0 --rexi-m='+str(m)+' -C '+str(-dt),	'REXI PAR M='+str(m),		comp_rexi_par,	'rexi_par_m'])

#	tests.append(['rexi_fd_m'+str(m).zfill(4),	' -S 0 --use-specdiff-for-complex-array 0 --rexi-h 0.8 --timestepping-mode 1 --staggering 0 --rexi-m='+str(m)+' -C '+str(-dt),	'REXI FD M='+str(m),		comp_rexi,	'rexi_fd_m'])
	tests.append(['rexi_fd_par_m'+str(m).zfill(4),	' -S 0 --use-specdiff-for-complex-array 0 --rexi-h 0.8 --timestepping-mode 1 --staggering 0 --rexi-m='+str(m)+' -C '+str(-dt),	'REXI PAR FD M='+str(m),	comp_rexi_par,	'rexi_fd_par_m'])


