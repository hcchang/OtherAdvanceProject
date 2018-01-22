
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <Cl/cl.h>
#include <fstream>
#include <string>

void medianFilterCPU(cv::Mat& imgN, cv::Mat& imgF);
void medianFilterCPUIt(cv::Mat& imgN, cv::Mat& imgF);
void swap(int* A, int* B);
void bubbleSort(int* kArray, int size);
int calcMedian(int* kArray, int size);


int main(int argc, char** argv)
{

	//初始化GPU
	cl_int err;
	cl_uint num;
	err = clGetPlatformIDs(0, 0, &num);
	if (err != CL_SUCCESS){
		std::cerr << "Unable to get platforms\n";
		return 0;
	}

	std::vector<cl_platform_id> platforms(num);
	err = clGetPlatformIDs(num, &platforms[0], &num);
	if (err != CL_SUCCESS){
		std::cerr << "Unable to get platform ID";
		return 0;
	}
	/////ask platform id//////////

	////get Platform and get Device
	////and get Command_queue
	cl_context_properties prop[] = { CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(platforms[0]), 0 }; 
	cl_context context = clCreateContextFromType(prop, CL_DEVICE_TYPE_GPU, NULL, NULL, NULL);
	if (context == 0){
		std::cerr << "Can't create OpenCL context\n";
		return 0;
	}

	size_t cb;
	clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &cb);
	std::vector<cl_device_id> devices(cb / sizeof(cl_device_id));
	clGetContextInfo(context, CL_CONTEXT_DEVICES, cb, &devices[0], 0);  ////get context

	clGetDeviceInfo(devices[0], CL_DEVICE_NAME, 0, NULL, &cb);
	std::string devname;
	devname.resize(cb);
	clGetDeviceInfo(devices[0], CL_DEVICE_NAME, cb, &devname[0], 0);    ////get device
	std::cout << "Device: " << devname.c_str() << std::endl;

	cl_uint compute_unit;
	clGetDeviceInfo(devices[0], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &compute_unit, 0);
	std::cout << "compute_unit: " << compute_unit << std::endl;

	cl_uint max_work_group_size;
	clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(cl_uint), &max_work_group_size, 0);
	std::cout << "max_work_group_size: " << max_work_group_size << std::endl;

	cl_command_queue queue = clCreateCommandQueue(context, devices[0], 0, 0);
	if (queue == 0){
		std::cerr << "Can't create command queue" << std::endl;
		clReleaseContext(context);
		return 0;
	}
	//初始化GPU 完成

	//讀取cl的程式碼，需要給GPUcompile
	std::string clFileName = "MedianFilter.cl";

	std::ifstream clFile(clFileName, std::ios_base::binary);
	if (!clFile.good())
	{
		std::cout << "無法讀取檔案" << std::endl;
		return -1;
	}
	// 取得檔案長度
	clFile.seekg(0, std::ios_base::end);
	size_t codeLength = clFile.tellg();
	clFile.seekg(0, std::ios_base::beg);
	//read program source
	std::vector<char> fileCodeData(codeLength + 1);
	clFile.read(&fileCodeData[0], codeLength);
	fileCodeData[codeLength] = 0;

	//create and bild program
	cl_build_status status;
	size_t logsize;
	const char* source = &fileCodeData[0];
	char* programLog;

	cl_program _program = clCreateProgramWithSource(context, 1, &source, 0, 0);
	err = clBuildProgram(_program, 0, 0, 0, 0, 0);

	//以下部分會顯示cl內錯誤的程式碼
	if (err != CL_SUCCESS)
	{
		clGetProgramBuildInfo(_program, devices[0], CL_PROGRAM_BUILD_STATUS, sizeof(cl_build_status), &status, NULL);
		clGetProgramBuildInfo(_program, devices[0], CL_PROGRAM_BUILD_LOG, 0, NULL, &logsize);
		programLog = (char*)calloc(logsize + 1, sizeof(char));
		clGetProgramBuildInfo(_program, devices[0], CL_PROGRAM_BUILD_LOG, logsize + 1, programLog, NULL);
		printf("Build failed; error = %d, status = %d, programLof = nn%s", err, status, programLog);
		free(programLog);

		return -1;
	}
	

	//讀取圖片
	cv::Mat imgN = cv::imread("ImgWithPepperSaltNoise.bmp", 0);

	if(imgN.empty())
	{
		std::cout << "無法讀取圖片" << std::endl;
		return -1;
	}

	cv::Mat imgF(cv::Size(imgN.cols, imgN.rows), CV_8UC1, cv::Scalar(0));
	int imgSize = imgN.cols*imgN.rows;
	cl_mem cl_ImgN = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_uchar)*imgSize, imgN.data, NULL);
	cl_mem cl_ImgF = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_uchar)*imgSize, imgF.data, NULL);

	float t1 = cv::getTickCount();

	cl_kernel MedianFilter = clCreateKernel(_program, "MedianFilter", 0);

	for (int i = 0; i < 10; i++)
	{
		
		clSetKernelArg(MedianFilter, 0, sizeof(cl_mem), &cl_ImgN);
		clSetKernelArg(MedianFilter, 1, sizeof(cl_mem), &cl_ImgF);

		const int workDimension = 2;
		size_t global_work_size[workDimension] = { imgN.cols, imgN.rows };

		cl_event evt;

		err = clEnqueueNDRangeKernel(queue, MedianFilter, 2, NULL, global_work_size, NULL, 0, NULL, &evt);

		if (err == CL_SUCCESS)
		{
			err = clEnqueueReadBuffer(queue, cl_ImgF, CL_TRUE, 0, sizeof(cl_uchar)*imgSize, imgF.data, 0, 0, 0);
		}

	}

	float t2 = 1000 * (cv::getTickCount() - t1) / cv::getTickFrequency();

	std::cout << "time: " << t2 << "ms" << std::endl;

	cv::imshow("MedianFilterOpenCL", imgF);

	cv::waitKey(0);

	cv::Mat imgFC(cv::Size(imgN.cols, imgN.rows), CV_8UC1, cv::Scalar(0));

	float t3 = cv::getTickCount();

	for (int i = 0; i < 10; i++)
	{
		//cv::medianBlur(imgN, imgFOpenCV, 5);
		medianFilterCPU(imgN, imgFC);
	}

	float t4 = 1000 * (cv::getTickCount() - t3) / cv::getTickFrequency();

	std::cout << "time: " << t4 << "ms" << std::endl;

	cv::imshow("MedianFilterOpenCV", imgFC);

	cv::waitKey(0);


	cv::Mat imgFCIt(cv::Size(imgN.cols, imgN.rows), CV_8UC1, cv::Scalar(0));

	float t5 = cv::getTickCount();

	for (int i = 0; i < 10; i++)
	{
		//cv::medianBlur(imgN, imgFOpenCV, 5);
		medianFilterCPUIt(imgN, imgFCIt);
	}

	float t6 = 1000 * (cv::getTickCount() - t5) / cv::getTickFrequency();

	std::cout << "time: " << t6 << "ms" << std::endl;

	cv::imshow("MedianFilterOpenCV", imgFCIt);

	cv::waitKey(0);

	clReleaseKernel(MedianFilter);
	clReleaseProgram(_program);
	clReleaseCommandQueue(queue);
	clReleaseContext(context);
	clReleaseMemObject(cl_ImgN);
	clReleaseMemObject(cl_ImgF);


	return 0;
}

void medianFilterCPU(cv::Mat& imgN, cv::Mat& imgF)
{

	int m_width = imgN.cols;
	int m_height = imgN.rows;	

	for (int oi = 2; oi < imgN.rows-2; oi++)
	{
		for (int oj = 2; oj < imgN.cols-2; oj++)
		{
			int index_k = 0;
			int kArray[25];
			for (int i = oi-2; i < 3 + oi; i++)
			{
				for (int j = oj-2; j < 3 + oj; j++)
				{
					kArray[index_k] = imgN.at<unsigned char>(i,j);
					index_k++;
				}
			}

			int median = calcMedian(kArray, 25);
			imgF.at<unsigned char>(oi, oj) = median;

		}
	}

}


void medianFilterCPUIt(cv::Mat& imgN, cv::Mat& imgF)
{

	cv::MatIterator_<unsigned char> it, end;

	int m_width = imgN.cols;
	int m_height = imgN.rows;

	unsigned char *pF;

	unsigned char *pN;
	for (int oi = 0; oi < imgN.rows; oi++)
	{
		pF = imgF.ptr<unsigned char>(oi);
		for (int oj = 0; oj < imgN.cols; oj++)
		{
			int index_k = 0;
			int kArray[25];

			for (int i = oi - 2; i < 3 + oi; i++)
			{
				pN = imgN.ptr<unsigned char>(i);
				for (int j = oj - 2; j < 3 + oj; j++)
				{
					kArray[index_k] = pN[j];
					index_k++;
				}
			}

			int median = calcMedian(kArray, 25);
			pF[oj] = median;		
		}
	}

}


void swap(int* A, int* B)
{
	int tmp;
	tmp = *A;
	*A = *B;
	*B = tmp;
}

void bubbleSort(int* kArray, int size)
{
	for (int i = 0; i< size - 1; i++)
	{
		for (int j = 0; j < size - i; j++)
		{
			if (kArray[j] > kArray[j + 1])
			{
				swap(&kArray[j], &kArray[j + 1]);
			}
		}
	}
}

int calcMedian(int* kArray, int size)
{
	int median;

	bubbleSort(kArray, size);

	if (size % 2 == 0)
	{
		median = (kArray[size / 2 - 1] + kArray[size / 2]) / 2;
	}
	else
	{
		median = kArray[(size - 1) / 2];
	}

	return median;
}
