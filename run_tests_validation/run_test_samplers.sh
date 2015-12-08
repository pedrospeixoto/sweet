#! /bin/bash


echo "***********************************************"
echo "Running tests for sampler operations"
echo "***********************************************"

cd ..

echo
echo "***********************************************"
echo "TEST SPECTRAL OPS (release) ALIASING CONTROL $X"
echo "***********************************************"
make clean
scons --unit-test=test_samplers --gui=disable
./build/test_samplers_libfft_omp_gnu_release -s 2 || exit
./build/test_samplers_libfft_omp_gnu_release -s 3 || exit



echo "***********************************************"
echo "***********************************************"
echo "***********************************************"
echo "***********************************************"
echo "***************** FIN *************************"
echo "***********************************************"
echo "***********************************************"
echo "***********************************************"
echo "***********************************************"
