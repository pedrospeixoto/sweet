#! /bin/bash


. /etc/profile.d/modules.sh

module unload gcc
module unload fftw

module unload python
module load python/2.7_anaconda_nompi


module unload intel
module load intel/16.0

module unload mpi.intel
module load mpi.intel/5.1

module load gcc/5

cd /home/martin/workspace/sweet/benchmarks/rexi_tests_lrz_freq_waves/2015_12_27_scalability_rexi_fd
cd ../../../

. local_software/env_vars.sh



scons --compiler=intel --sweet-mpi=enable --program=swe_rexi --spectral-space=disable --libfft=enable --rexi-parallel-sum=disable --spectral-dealiasing=disable --mode=release --numa-block-allocator=1  --threading=omp --program-binary-name=rexi_m_tyes_a1 || exit 1
