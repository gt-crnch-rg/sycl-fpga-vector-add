# Add header files
file(GLOB H_FILES ${PROJECT_SOURCE_DIR}/include/*.hpp)

# Specify the executable and source files
file(GLOB alg_src_files "${PROJECT_SOURCE_DIR}/src/algebra_ops.cpp")
file(GLOB vadd_src_files "${PROJECT_SOURCE_DIR}/src/fpga_vector_add.cpp")

#Algebra ops makes use of threading and pthreads
#Compiling without pthread linking will result in the error:
#undefined reference to symbol 'pthread_create@@GLIBC_2.2.5'
#/lib/x86_64-linux-gnu/libpthread.so.0: error adding symbols: DSO missing from command line
set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)
#Alternatively you can have CMake search for threads/pthreads
#set (THREADS_PREFER_PTHREAD_FLAG ON)
#find_package (Threads REQUIRED)
#target_link_libraries(${alg_trgt} PRIVATE Threads::Threads)

add_executable(${alg_trgt} ${alg_src_files} ${H_FILES})
add_executable(${vadd_trgt} ${vadd_src_files} ${H_FILES})
target_link_libraries(${vadd_trgt} LINK_PUBLIC OpenCL sycl)
#Prefer C11 standard to allow for loop initialization and aligned_alloc
set_property(TARGET ${vadd_trgt} PROPERTY C_STANDARD 11)

#Copy the executable to the base build directory. This could be changed
#based on deployment
#install(PROGRAMS ${alg_trgt} DESTINATION ${CMAKE_BINARY_DIR})
