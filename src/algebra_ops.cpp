/**
 * SYCL Algebra Operations Tests
 * Designed to compare performance of SYCL with standard multi-threaded approaches.
 * @author James Wood
 *
 * To compile, use clang++ -pthread -fsycl algebra_ops.cpp -o algebra_ops.exe
 * Takes command-line arguments -c (count of elements) and -t (number of threads)
 */

#include <CL/sycl.hpp>
#include <random>
#include <chrono>
#include <thread>
#include <cmath>
#include <algorithm>
#include <unistd.h>

#include "../include/sycl_dev_profile.hpp"

using namespace cl::sycl;

class vector_add;
class matrix_vector_mult;

/*Adds two vectors, v1 and v2, and places the result into v3.
 *v1, v2, and v3 must all be the same size for intended functionality.
 *Performs this operation single-threaded.
 *Returns the amount of time in microseconds that this operation took.
 */
unsigned int vector_addition(float* v1, float* v2, float* v3, unsigned int size)
{
	auto start = std::chrono::high_resolution_clock::now();
	for (unsigned int i = 0; i < size; i++)
	{
		v3[i] = v1[i] + v2[i];
	}
	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	return duration.count();
}

void vector_addition_multi_helper(float*, float*, float*, unsigned int, unsigned int);

/*Adds two vectors, v1 and v2, and places the result into v3.
 * v1, v2, and v3 must all be the same size for intended functionality.
 * Performs this operation multi-threaded.
 * Returns the amount of time in microseconds that this operation took.
 * Uses a specified number of threads in thread_count to complete the operation.
 */
unsigned int vector_addition_multi(float* v1, float* v2, float* v3, unsigned int size, unsigned int thread_count)
{
	auto start = std::chrono::high_resolution_clock::now();
	int division_size = size / thread_count;
	std::vector<std::thread> threads;
	
	for (int i = 0; i < thread_count; i++)
	{
		if (i != thread_count - 1)
		{
			threads.push_back(std::thread(vector_addition_multi_helper, v1, v2, v3, i * division_size, (i + 1) * division_size));
		}
		else
		{
			threads.push_back(std::thread(vector_addition_multi_helper, v1, v2, v3, i * division_size, size));
		} 
	}	

	for (int j = 0; j < thread_count; j++)
	{
		threads[j].join();
	}

	auto stop = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
	return duration.count();
}

void vector_addition_multi_helper(float* v1, float* v2, float* v3, unsigned int startIndex, unsigned int stopIndex)
{
	for (unsigned int i = startIndex; i < stopIndex; i++)
	{
		v3[i] = v1[i] + v2[i];
	}
}

/*Adds two vectors, v1 and v2, and places the result into v3.
 * v1, v2, and v3 must all be the same size for intended functionality.
 * Performs this operation using SYCL on CPU.
 * Returns the amount of time in microseconds that this operation took.
 * Uses a specified number of threads in thread_count to complete the operation.
 */
unsigned int vector_addition_sycl(float* v1, float* v2, float* v3, unsigned int size, unsigned int thread_count)
{
	default_selector deviceSelector{};
	property_list propList{property::queue::enable_profiling()};

	queue queue{deviceSelector, propList};
	std::cout << "SYCL Running on " << queue.get_device().get_info<info::device::name>() << std::endl;
	
	std::vector<event> eventList;
	std::vector<std::chrono::time_point<std::chrono::high_resolution_clock>> startTimeList;

	{
		startTimeList.push_back(std::chrono::high_resolution_clock::now());

		buffer<float, 1> bufferV1(v1, range<1>(size));
		buffer<float, 1> bufferV2(v2, range<1>(size));
		buffer<float, 1> bufferV3(v3, range<1>(size));
		
		unsigned int division_size = size / thread_count;

		eventList.push_back(queue.submit([&](handler &cgh)
		{
			auto in1 = bufferV1.get_access<access::mode::read>(cgh);
			auto in2 = bufferV2.get_access<access::mode::read>(cgh);
			auto in3 = bufferV3.get_access<access::mode::write>(cgh);

			cgh.parallel_for<vector_add>(range<1>(thread_count),[=](id<1> i)
			{
				for (unsigned int j = i * division_size; j < (i + 1) * division_size && j < size; j++) 
				{
					in3[j] = in1[j] + in2[j];
				}
			});
		}));
	}	

	example_profiler<double> profiler(eventList, startTimeList);
	return (unsigned int) profiler.get_kernel_execution_time();
}

/*Generates a 1-dimensional vector of floats of length size.
 * Initializes this vector with random values.
 * Will push this vector onto the back of vectors.
 */
void GenerateRandomFloatVector(std::mt19937 gen, std::uniform_real_distribution<float> distribution, unsigned int size, std::vector<std::vector<float>*>* vectors)
{
	std::vector<float>* temp = new std::vector<float>;

	for (auto i = 0; i < size; i++)
	{
		temp->push_back(distribution(gen));
	}

	vectors->push_back(temp);
}

void CompareVectorResultsHelper(std::vector<std::vector<float>*>, unsigned int, unsigned int, bool*);

/*Compares the given vectors in vectors and returns false if they are not all equal.
 */
bool CompareVectorResults(std::vector<std::vector<float>*> vectors)
{
	unsigned int thread_count = std::thread::hardware_concurrency() / 2;
	if (thread_count < 1)
		thread_count = 1;
	unsigned int division_size = vectors[0]->size() / thread_count;
	std::thread threads[thread_count];
	bool results[thread_count];

	for (auto i = 0; i < thread_count; i++)
	{
		results[i] = true;
		threads[i] = std::thread(CompareVectorResultsHelper, vectors, i * division_size, (i + 1) * division_size, &results[i]); 
	}

	bool returnValue = true;
	for (auto j = 0; j < thread_count; j++)
	{
		threads[j].join();
		if (!results[j])
		{
			returnValue = false;
		}
	}

	return returnValue;
}

void CompareVectorResultsHelper(std::vector<std::vector<float>*> vectors, unsigned int startIndex, unsigned int stopIndex,  bool* result)
{
	bool shouldContinue = true;
	for (auto i = startIndex; i < stopIndex && i < vectors[0]->size() && shouldContinue; i++)
	{
		float currentValue = vectors[0]->data()[i];
		for (auto j = 1; j < vectors.size() && shouldContinue; j++)
		{
			if (abs(vectors[j]->data()[i] - currentValue) > 0.01)
			{
				shouldContinue = false;
				*result = false;
			}	
		}
	}
}

int main(int argc, char *argv[])
{
	int c;
	unsigned int vector_sizes = 1;
	unsigned int thread_count = 1;
	while ((c = getopt(argc, argv, "c:t:")) != -1)
	{
		switch (c)
		{
			case 'c':
				sscanf(optarg, "%u", &vector_sizes);
				break;
			case 't':
				sscanf(optarg, "%u", &thread_count);
				break;
			default:
				printf("Unrecognized argument(s). Exiting.\n");
				return 0;
		}		
	}

	printf("Using %u threads. Using %u elements.\n", thread_count, vector_sizes);

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> distribution(0.0, 1.0f);
	std::vector<std::vector<float>*> dataVectors;
	std::vector<std::vector<float>*> resultVectors;
	std::vector<std::thread> threads;

	for (int i = 0; i < 2; i++)
	{
		threads.push_back(std::thread(GenerateRandomFloatVector, gen, distribution, vector_sizes, &dataVectors));
	}

	for (int x = 0; x < 2; x++)
	{
		threads[x].join();
	}

	resultVectors.push_back(new std::vector<float>(vector_sizes));
	resultVectors.push_back(new std::vector<float>(vector_sizes));
	resultVectors.push_back(new std::vector<float>(vector_sizes));

	float* a_data = dataVectors[0]->data();
	float* b_data = dataVectors[1]->data();
	float* c_data = resultVectors[0]->data();
	float* d_data = resultVectors[1]->data();
	float* e_data = resultVectors[2]->data();

	unsigned int duration1 = vector_addition(a_data, b_data, c_data, dataVectors[0]->size());
	unsigned int duration2 = vector_addition_sycl(a_data, b_data, d_data, dataVectors[0]->size(), thread_count);
	unsigned int duration3 = vector_addition_multi(a_data, b_data, e_data, dataVectors[0]->size(), thread_count);

	if (!CompareVectorResults(resultVectors))
	{
		printf("Results from each run are not consistent!\n");
		return 0;
	}

	printf("Note: All durations are in microseconds.\n");
	printf("Duration Single-Threaded: %u\nDuration CPU SYCL: %u\nDuration Multi-Threaded: %u\n", duration1, duration2, duration3);

	return 0;
}
