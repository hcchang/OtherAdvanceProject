#include <iostream>
#include <CL\cl.h>
#include <vector>


int main(int argc, char** argv)
{
	cl_int err_num;
	cl_uint num_platforms;

	err_num = clGetPlatformIDs(0, 0, &num_platforms);
	if (err_num != CL_SUCCESS)
	{
		std::cout << "can't get platforms" << std::endl;
		return 0;
	}

	std::cout << "platform numbers: " << num_platforms << std::endl;

	std::vector<cl_platform_id> platformsIds(num_platforms);
	err_num = clGetPlatformIDs(num_platforms, &platformsIds[0], &num_platforms);
	if (err_num != CL_SUCCESS)
	{
		std::cout << "can't get platforms IDs" << std::endl;
		return 0;
	}

	std::cout << "how many platform ids: " << platformsIds.size() << std::endl;

	cl_context_properties properties[] = { CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(platformsIds[0]), 0 }; // nVidia
	cl_context context = clCreateContextFromType(properties, CL_DEVICE_TYPE_GPU, NULL, NULL, NULL);
	if (context == 0){
		std::cout << "can't create OpenCL context" << std::endl;
		return 0;
	}

	size_t param_value_size;
	clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &param_value_size);
	std::vector<cl_device_id> devices(param_value_size / sizeof(cl_device_id));
	clGetContextInfo(context, CL_CONTEXT_DEVICES, param_value_size, &devices[0], 0);  ////get context


	char device_string[1024];
	char dname[500];
	size_t workitem_size[3];

	cl_device_id device = devices[0];

	clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_string), &device_string, NULL);
	std::cout << "DEVICE: " << device_string << std::endl;

	// CL_DEVICE_INFO
	cl_device_type type;
	clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(type), &type, NULL);
	if (type & CL_DEVICE_TYPE_CPU)
		std::cout << "CL_DEVICE_TYPE: " << "CL_DEVICE_TYPE_CPU" << std::endl;
	if (type & CL_DEVICE_TYPE_GPU)
		std::cout << "CL_DEVICE_TYPE: " << "CL_DEVICE_TYPE_GPU" << std::endl;
	if (type & CL_DEVICE_TYPE_ACCELERATOR)
		std::cout << "CL_DEVICE_TYPE: " << "CL_DEVICE_TYPE_ACCELERATOR" << std::endl;
	if (type & CL_DEVICE_TYPE_DEFAULT)
		std::cout << "CL_DEVICE_TYPE: " << "CL_DEVICE_TYPE_DEFAULT" << std::endl;

	clGetDeviceInfo(device, CL_DRIVER_VERSION, 500, dname, NULL);
	std::cout << "Driver version = : " << dname << std::endl;

	cl_ulong long_entries;
	clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &long_entries, NULL);
	std::cout << "Global Memory (MB): " << long_entries / 1024 / 1024 << std::endl;

	clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_CACHE_SIZE, sizeof(cl_ulong), &long_entries, NULL);
	std::cout << "Global Memory Cache (B): " << long_entries << std::endl;

	clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &long_entries, NULL);
	std::cout << "Local Memory (B): " << long_entries << std::endl;

	clGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_ulong), &long_entries, NULL);
	std::cout << "CL_DEVICE_MAX_CLOCK_FREQUENCY: " << long_entries << " GHz" << std::endl;

	cl_uint compute_units;
	clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, NULL);
	std::cout << "CL_DEVICE_MAX_COMPUTE_UNITS: " << compute_units << std::endl;

	clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(workitem_size), &workitem_size, NULL);
	std::cout << "CL_DEVICE_MAX_WORK_ITEM_SIZES: " << workitem_size[0] << "x" << workitem_size[1] << "x" << workitem_size[2] << std::endl;

	return 0;
}