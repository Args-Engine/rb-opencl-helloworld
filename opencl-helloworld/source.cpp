

#include <CL/cl.h>
#include <iostream>
#include <array>
#include <vector>
#include <iomanip>

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>

void print_success();

void print_failure();


#define CHECK_RET(X) if ((X) != CL_SUCCESS) { std::cout << #X; print_failure() ;} else { std::cout << #X; print_success() ;}
#define CHECK_NULL(X,MSG) if ((X) == NULL) { std::cout << (MSG); print_failure(); } else { std::cout << (MSG); print_success() ;}

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

int main()
{
	SetConsoleTextAttribute(hConsole, 7);

	cl_platform_id platform;

	CHECK_RET(clGetPlatformIDs(1, &platform, nullptr))

	cl_device_id device;

	CHECK_RET(clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, nullptr))


	cl_int ret_;

	cl_context context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &ret_);
	CHECK_NULL(context, "createContext")



	constexpr int LIST_SZ = 524288;

	std::vector<int> lhs(LIST_SZ);
	std::vector<int> rhs(LIST_SZ);
	std::vector<int> res(LIST_SZ);

	for(int i = 0; i < LIST_SZ; i++)
	{
		lhs[i] = i;
		rhs[i] = LIST_SZ - i;
	}
	

	FILE* fp;
	fp = std::fopen("vadd.cl", "r");
	CHECK_NULL(fp, "fopen")

	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	std::vector<char> source(size);

	fread(source.data(), 1, source.size(), fp);

	fclose(fp);
	const char* src = source.data();
	const size_t src_len = source.size();


	char version[256];
	clGetPlatformInfo(platform, CL_PLATFORM_VERSION, 256, &version, nullptr);

	std::cout << "CL ApparentVersion: " << version << std::endl;

	cl_program program = clCreateProgramWithSource(context, 1, &src, &src_len, nullptr);
	CHECK_NULL(program, "clCreateProgramWithSource")
	CHECK_RET(clBuildProgram(program, 1, &device, " -cl-std=CL2.0", nullptr, nullptr))
	std::cout << "CL MeasuredVersion: OpenCL 2.0" << std::endl;

	cl_kernel kernel = clCreateKernel(program, "vector_add", NULL);
	CHECK_NULL(kernel, "clCreateKernel")

	cl_mem lhs_mem,rhs_mem,ret_mem;

	lhs_mem = clCreateBuffer(context,CL_MEM_READ_ONLY, LIST_SZ * sizeof(int),nullptr,&ret_);
	rhs_mem = clCreateBuffer(context,CL_MEM_READ_ONLY, LIST_SZ * sizeof(int),nullptr,&ret_);
	ret_mem = clCreateBuffer(context,CL_MEM_WRITE_ONLY,LIST_SZ * sizeof(int),nullptr,&ret_);

	CHECK_NULL(lhs_mem && rhs_mem && ret_mem,"clCreateBuffer(s)")

	cl_command_queue queue = clCreateCommandQueueWithProperties(context,device,nullptr,&ret_);
	CHECK_NULL(queue,"clCreateCommandQueueWithProperties")

	CHECK_RET(clSetKernelArg(kernel,0,sizeof(cl_mem),&lhs_mem))
	CHECK_RET(clSetKernelArg(kernel,1,sizeof(cl_mem),&rhs_mem))
	CHECK_RET(clSetKernelArg(kernel,2,sizeof(cl_mem),&ret_mem))

	CHECK_RET(clEnqueueWriteBuffer(queue,lhs_mem,CL_TRUE,0,LIST_SZ * sizeof(int),lhs.data(),0,NULL,NULL))
	CHECK_RET(clEnqueueWriteBuffer(queue,rhs_mem,CL_TRUE,0,LIST_SZ * sizeof(int),rhs.data(),0,NULL,NULL))
	
	size_t globals = LIST_SZ;
	size_t locals = 1024;
	
	CHECK_RET(clEnqueueNDRangeKernel(queue,kernel,1,NULL,&globals,&locals,0,NULL,NULL))
	CHECK_RET(clEnqueueReadBuffer (queue,ret_mem,CL_TRUE,0,LIST_SZ * sizeof(int),res.data(),0,NULL,NULL))

	CHECK_RET(clFlush(queue))


	SetConsoleTextAttribute(hConsole,10);
	
	for(int i = 0; i < LIST_SZ; i+= 1)
	{
		if( res[i] != LIST_SZ) std::cout << "failure" <<std::endl;
	}

	SetConsoleTextAttribute(hConsole,7);

	
	ret_ |= clFinish(queue);
	ret_ |= clReleaseKernel(kernel);
	ret_ |= clReleaseProgram(program);
	ret_ |= clReleaseMemObject(lhs_mem);
	ret_ |= clReleaseMemObject(rhs_mem);
	ret_ |= clReleaseMemObject(ret_mem);
	ret_ |= clReleaseCommandQueue(queue);
	ret_ |= clReleaseContext(context);
	CHECK_RET(ret_)
}



void print_failure()
{
	SetConsoleTextAttribute(hConsole, 12);

	std::printf(" [failure]\n");

	SetConsoleTextAttribute(hConsole, 7);

	std::exit(1);
}


void print_success()
{
	SetConsoleTextAttribute(hConsole, 10);

	std::printf(" [success]\n");


	SetConsoleTextAttribute(hConsole, 7);
}
