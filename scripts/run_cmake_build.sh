#!/bin/bash
#Just shows how to use CMake with newer syntax
#-B specifies build directory, -S specifies source directory

#Run from the main directory. Ex: ./scripts/run_cmake_build.sh

#We want to test each of the build options:
for FPGA_STRING in CPU_HOST USE_FPGA_EMULATOR USE_FPGA_REPORT USE_FPGA_BITSTREAM USE_FPGA_PROFILING
do	
	#Use a case statement to specify the build directory based on the passed CMAKE flag
	case $FPGA_STRING in
		CPU_HOST )
			buildtype=cpu ;;
		USE_FPGA_EMULATOR )
			buildtype=fpga_emu ;;
		USE_FPGA_REPORT )
			buildtype=fpga_report ;;
		USE_FPGA_BITSTREAM )
			buildtype=fpga_bitstream ;;
		USE_FPGA_PROFILING )
			buildtype=fpga_profile ;;
	esac

	#Build, print out, and then execute the cmake command
	cmd="cmake -B oneapi_${buildtype} -S . -D$FPGA_STRING=1 -DCMAKE_TOOLCHAIN_FILE=cmake/oneapi-toolchain.cmake -DUSE_SYCL=1"
	echo ${cmd}
	${cmd}
done
