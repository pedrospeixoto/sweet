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

cd /home/hpc/pr63so/di69fol/workspace/SWEET_2015_12_26/benchmarks_performance/rexi_tests_lrz_freq_waves/2016_01_06_scalability_rexi_fd_run2
cd ../../../

. local_software/env_vars.sh



scons --compiler=intel --sweet-mpi=enable --program=swe_rexi --spectral-space=disable --libfft=enable --rexi-parallel-sum=enable --spectral-dealiasing=disable --mode=release --numa-block-allocator=1  --threading=off --program-binary-name=rexi_fd_par_m_tno_a1 || exit 1
