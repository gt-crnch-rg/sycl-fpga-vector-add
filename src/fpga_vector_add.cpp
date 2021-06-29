#include <vector>
#include <CL/sycl.hpp>
//#include "${ONEAPI_ROOT}/compiler/latest/linux/include/sycl/CL/sycl/INTEL/fpga_extensions.hpp"
#include <CL/sycl/INTEL/fpga_extensions.hpp>
#include <chrono>
#include <fstream>

#include "../include/sycl_dev_profile.hpp"

using namespace cl::sycl;

class vector_add;

int main() {

  //Block off this code
  //Putting all SYCL work within here ensures it concludes before this block
  //  goes out of scope. Destruction of the buffers is blocking until the
  //  host retrieves data from the buffer.
  {

    property_list pl{sycl::property::queue::enable_profiling()};
  
    //Device selection
    //We will explicitly compile for the FPGA_EMULATOR, CPU_HOST, or FPGA
    #if defined(FPGA_EMULATOR)
      std::cout << "Using FPGA Emulator." << std::endl;
      INTEL::fpga_emulator_selector device_selector;
    #elif defined(CPU_HOST)
      std::cout << "Using CPU Host." << std::endl;
      host_selector device_selector;
    #else
      std::cout << "Using FPGA hardware (bitstream)." << std::endl;
      INTEL::fpga_selector device_selector;
    #endif

    //Create queue
    queue device_queue(device_selector,NULL,pl);
  
    //Query platform and device
    platform platform = device_queue.get_context().get_platform();
    sycl::device device = device_queue.get_device();
    std::cout << "Platform name: " <<  platform.get_info<sycl::info::platform::name>().c_str() << std::endl;
    std::cout << "Device name: " <<  device.get_info<sycl::info::device::name>().c_str() << std::endl;
      
    std::vector<event> eventList;
    std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>> startTimeList;
      
    std::vector<float> v1, v2, v3;
    
    for (int x = 0; x < 1000000; x++)
    {
        v1.push_back(1.0f);
        v2.push_back(2.0f);
        v3.push_back(0.0f);
    }
      
    // Defining the buffers, which take in the type of data, the dimension of the data, a pointer to the data, and the number of elements
    buffer<float, 1> bufV1(v1.data(), v1.size());
    buffer<float, 1> bufV2(v2.data(), v2.size());
    buffer<float, 1> bufV3(v3.data(), v3.size());

    startTimeList.push_back(std::chrono::high_resolution_clock::now());
      
    //Iteration size for the kernel
    //Marking this a const on purpose, so that the kernel can use it easily. 
    const int kernel_iter = v1.size();

    //Device queue submit
    eventList.push_back(device_queue.submit([&](sycl::handler &cgh) 
    {
    
      //Create accessors, so that we can use the buffers within our kernel
      //Note that there are different access modes, like read, write, and read/write
      auto in1 = bufV1.get_access<access::mode::read>(cgh);
      auto in2 = bufV2.get_access<access::mode::read>(cgh);
      auto out = bufV3.get_access<access::mode::write>(cgh);

      //Call the kernel
      // We will define it within this using, again, kinda weird syntax
      // Also note the use of "single_task" which is good for hardware like FPGAs, and not so good for GPUs
      // There are a variety of options available, such as "parallel_for" which is good for GPUs but needs different syntax
      cgh.single_task<vector_add>([=]() 
      {
         // Note that we manipulate the data through the use of the accessors
         // This will automatically update on the host system once the kernel is done running
         for (int i = 0; i < kernel_iter; i++)
         {
            out[i] = in1[1] + in2[2];
         }
          
      });
  
    }));

    //Wait for the kernel to get finished before reporting the profiling
    device_queue.wait();
    example_profiler<double> profiler(eventList, startTimeList);
    std::cout << "Kernel Execution Time: " << profiler.get_kernel_execution_time() << std::endl;
      
    // Verification that the operations were correct
    for (int y = 0; y < v1.size(); y++)
    {
        if((v3[y] - (v1[y] + v2[y])) > 0.1f)
        {
            std::cout << "Results not consistent!" << std::endl;
            return 0;
        }
    }
  }

  return 0;
}
