#!/bin/bash
#Just shows how to use CMake with newer syntax to compile

#SYCL Algebra will likely only work with CPU host
for FPGA_STRING in CPU_HOST
do	
	bld_dir=oneapi_cpu
	cmd="cmake --build ${bld_dir} --config Release --target sycl_algebra"
	echo ${cmd}
	${cmd}
done

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

	bld_dir=oneapi_${buildtype}

	#Use a case statement to specify the build directory based on the passed CMAKE flag
	cmd="cmake --build ${bld_dir} --config Release --target vadd"
	echo ${cmd}
	${cmd}
done
