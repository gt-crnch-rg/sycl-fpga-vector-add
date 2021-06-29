#3.18 is the latest cmake release
cmake_minimum_required (VERSION 3.10 FATAL_ERROR)

#Define the project
project(sycl_cmake VERSION 0.5 LANGUAGES CXX)

# Set variables for target executables
set (alg_trgt sycl_algebra)
set (vadd_trgt vadd)



#Set flags when -D USE_SYCL is set
IF (USE_SYCL)
    ADD_DEFINITIONS (-DUSE_SYCL)
    IF (USE_FPGA_EMULATOR)
	SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fintelfpga")
	ADD_DEFINITIONS (-DFPGA_EMULATOR)
    ELSEIF (USE_FPGA_REPORT)
	SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fintelfpga -fsycl-link -Xshardware")
	ADD_DEFINITIONS (-DFPGA_EMULATOR)
    ELSEIF (USE_FPGA_BITSTREAM)
	SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fintelfpga -Xshardware")
    ELSEIF (USE_FPGA_PROFILING)
	SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fintelfpga -Xshardware -Xsprofile")
    ELSE () #Use CPU emulation by default
	message("Building for CPU host emulation")
	ADD_DEFINITIONS (-DCPU_HOST)
    ENDIF ()

    SET (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${ONEAPI_BASE}/compiler/latest/linux/include/sycl/")
    message(${CMAKE_CXX_FLAGS})
    #Alternate way to include a pointer to the directory with fpga_extensions.hpp
    #SET (CPATH "${ONEAPI_BASE}/compiler/latest/linux/include/sycl/")
ENDIF ()

#Collect all the source files - exe is specified in the sub-directory
FILE (GLOB H_FILES include/*.hpp)
add_subdirectory(src)

# This section shows how to use CPACK to create a tarball for your codebase
IF (RUN_CPACK)
	#Set a tool version and add CPack support to build a binary
	#set(Tool_VERSION_MAJOR "0")
	#set(Tool_VERSION_MINOR "5")

	#Include needed system libraries
	include(InstallRequiredSystemLibraries)
	set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
	#set(CPACK_PACKAGE_VERSION_MAJOR "${Tool_VERSION_MAJOR}")
	#set(CPACK_PACKAGE_VERSION_MINOR "${Tool_VERSION_MINOR}")
	set(CPACK_GENERATOR TGZ)
	include(CPack)
ENDIF ()
