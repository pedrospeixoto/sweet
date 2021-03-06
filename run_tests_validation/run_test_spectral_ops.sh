#! /bin/bash


echo "***********************************************"
echo "Running tests for Spectral operations"
echo "***********************************************"

# set close affinity of threads
export OMP_PROC_BIND=close


# 10^4 km
MAX_SCALE=$((10000*1000))
MIN_SCALE=0.01


echo "MAX/MIN SCALE: $MAX_SCALE / $MIN_SCALE"

cd ..
if true; then
	X=$MAX_SCALE
	Y=$MIN_SCALE
	echo
	echo "***********************************************"
	echo "TEST SPECTRAL OPS (release) $X x $Y"
	echo "***********************************************"
	make clean
	scons --threading=omp --unit-test=test_spectral_ops --gui=disable --spectral-space=enable --mode=release --spectral-dealiasing=disable
	EXEC="./build/test_spectral_ops_spectral_libfft_omp_gnu_release  -X $X -Y $Y -S 0"
	echo "$EXEC"
	$EXEC || exit
	EXEC="./build/test_spectral_ops_spectral_libfft_omp_gnu_release  -X $X -Y $Y -S 1"
	echo "$EXEC"
	$EXEC || exit
fi

X=$MAX_SCALE
echo
echo "***********************************************"
echo "TEST SPECTRAL OPS (release) $X"
echo "***********************************************"
make clean
scons --threading=omp --unit-test=test_spectral_ops --gui=disable --spectral-space=enable --mode=release --spectral-dealiasing=disable
EXEC="./build/test_spectral_ops_spectral_libfft_omp_gnu_release -n 128 -m 128 -X $X -Y $X -S 1"
echo "$EXEC"
$EXEC || exit
EXEC="./build/test_spectral_ops_spectral_libfft_omp_gnu_release -n 128 -m 128 -X $X -Y $X -S 0"
$EXEC || exit

X=$MIN_SCALE
echo
echo "***********************************************"
echo "TEST SPECTRAL OPS (release) $X"
echo "***********************************************"
make clean
scons --threading=omp --unit-test=test_spectral_ops --gui=disable --spectral-space=enable --mode=release --spectral-dealiasing=disable
./build/test_spectral_ops_spectral_libfft_omp_gnu_release -n 128 -m 128 -X $X -Y $X -S 1 || exit
./build/test_spectral_ops_spectral_libfft_omp_gnu_release -n 128 -m 128 -X $X -Y $X -S 0 || exit

X=$MAX_SCALE
echo
echo "***********************************************"
echo "TEST SPECTRAL OPS (release) ALIASING CONTROL $X"
echo "***********************************************"
make clean
scons --threading=omp --unit-test=test_spectral_ops --gui=disable --spectral-space=enable --mode=release --spectral-dealiasing=enable
./build/test_spectral_ops_spectral_libfft_dealiasing_omp_gnu_release -n 128 -m 128 -X $X -Y $X -S 0 || exit
./build/test_spectral_ops_spectral_libfft_dealiasing_omp_gnu_release -n 128 -m 128 -X $X -Y $X -S 1 || exit

X=$MIN_SCALE
echo
echo "***********************************************"
echo "TEST SPECTRAL OPS (release) ALIASING CONTROL $X"
echo "***********************************************"
make clean
scons --threading=omp --unit-test=test_spectral_ops --gui=disable --spectral-space=enable --mode=release --spectral-dealiasing=enable
./build/test_spectral_ops_spectral_libfft_dealiasing_omp_gnu_release -n 128 -m 128 -X $X -Y $X -S 0 || exit
./build/test_spectral_ops_spectral_libfft_dealiasing_omp_gnu_release -n 128 -m 128 -X $X -Y $X -S 1 || exit



echo "***********************************************"
echo "***********************************************"
echo "***********************************************"
echo "***********************************************"
echo "***************** FIN *************************"
echo "***********************************************"
echo "***********************************************"
echo "***********************************************"
echo "***********************************************"
