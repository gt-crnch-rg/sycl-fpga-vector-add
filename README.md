# sycl-fpga-vector-add

This project provides the following:
* A basic vector algebra implementation in SYCL and code for comparing with a multithreaded implementation
* An FPGA vector add geared towards use with Intel's OneAPI
* CMake examples of compilation for FPGA emulation and bitstreams tested with OneAPI 2021.2

# Compilation
Example (from within a directory inside of the repo)

```
. /tools/intel/oneapi/1.0/setvars.sh
#Source a newer CMake than the default one installed
. /tools/misc/env/cmake_build.sh
mkdir oneapi_build && cd oneapi_build
cmake .. -DCMAKE_TOOLCHAIN_FILE=cmake/oneapi-toolchain.cmake -DUSE_FPGA_EMULATOR=1 -DUSE_SYCL=1
```

CMake Flag Options
* `USE_SYCL` In order to compile with SYCL support (basically required).
* `USE_CPU_HOST` In order to compile to target the host device.
* `USE_FPGA_EMULATOR` In order to compile to target the FPGA emulator.
* `USE_FPGA_REPORT` In order to compile to target the FPGA emulator and generate a static report.
* `USE_FPGA_PROFILING` In order to compile to target FPGA with vTune profiling enabled.
* Specify none of the above options (except for `USE_SYCL`) in order to generate a bitstream for FPGA.

## Scripts
The `scripts` directory contains three example scripts that iterate over all the options (`CPU_HOST`, `EMULATOR`, etc.) to 1) create appropriate build directories, compile, and package a build directory. You might want to run one or two of these scripts just to see how the process works with CMake. Note that bitstream generation will take a while to complete.

```
. /tools/intel/oneapi/1.0/setvars.sh
#Source a newer CMake than the default one installed
. /tools/misc/env/cmake_build.sh
[sycl-testing]$ ./scripts/run_cmake_build.sh
...
#5 build directories should be created following the standard development flow
[sycl-testing]$ ls
cmake  CMakeLists.txt  include  License.txt  oneapi_cpu  oneapi_fpga_bitstream  oneapi_fpga_emu  oneapi_fpga_profile  oneapi_fpga_report  README.md  scripts  src
```
