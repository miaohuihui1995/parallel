#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <assert.h>
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>

#define OPENCL_CHECK_ERRORS(error)                                       \
	if(error != CL_SUCCESS){                                         \
		printf("errorcode: %d, at line %d\n", error, __LINE__);  \
		exit(1);                                                 \
	}                                                                

#define M 1024
#define P 512
#define N 2048

void RunAsCpu(const float *A, const float *B, float *C){
	for(int i = 0; i < M; i++){
		for(int j = 0; j < N; j++){
			C[i * N + j] = 0.0;
			for(int k = 0; k < P; k++){
				C[i * N + j] += A[i * P + k] * B[k * N + j];
			}
		}
	}
}

int main(int argc, const char **argv){

	cl_int error = 0;
	cl_context context;
	cl_command_queue queue;
	cl_device_id device;

	//遍历系统中所有OpenCL平台
	cl_uint num_of_platforms = 0;
	error = clGetPlatformIDs(0, 0, &num_of_platforms);
	OPENCL_CHECK_ERRORS(error);
	printf("可用平台数：%d\n", num_of_platforms);

	//得到所有平台的ID
	cl_platform_id *platforms = (cl_platform_id *)malloc(num_of_platforms * sizeof(cl_uint));
	error = clGetPlatformIDs(num_of_platforms, platforms, 0);
	OPENCL_CHECK_ERRORS(error);
	//遍历平台，选择一个Inter的平台
	cl_uint selected_platform_index = num_of_platforms;
	for(cl_uint i = 0; i < num_of_platforms; i++){
		size_t platform_name_length = 0;
		error = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 0, 0, &platform_name_length);
		OPENCL_CHECK_ERRORS(error);

		//调用两次，第一次是得到名称的长度
		char *platform_name = (char *)malloc(platform_name_length * sizeof(char));
		error = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, platform_name_length, platform_name, 0);
		OPENCL_CHECK_ERRORS(error);

		if(strstr(platform_name, "Intel(R) OpenCL") && selected_platform_index == num_of_platforms){
			printf("[%d], %s ", i, platform_name);
			printf("selected!\n");
			selected_platform_index = i;
		}
		free(platform_name);
		platform_name = NULL;
	}
	if(selected_platform_index == num_of_platforms){
		printf("没有找到Intel平台\n");
	}

	//Device
	cl_platform_id platform = platforms[selected_platform_index];
	error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	OPENCL_CHECK_ERRORS(error);

	//Context
	context = clCreateContext(0, 1, &device, NULL, NULL, &error);
	OPENCL_CHECK_ERRORS(error);

	//Command-queue CL_QUEUE_PROFILING_ENABLE开启才能计时
	queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &error);
	OPENCL_CHECK_ERRORS(error);

	//下面初始化测试数据（主机数据）
	float *A_h = (float *)malloc(M * P * sizeof(float));
	float *B_h = (float *)malloc(P * N * sizeof(float));
	float *C_h = (float *)malloc(M * N * sizeof(float));
	srand((unsigned)time(NULL));
	for(int i = 0; i < M * P; i++){
		A_h[i] = rand() % 50;
	}
	for(int i = 0; i < P * N; i++){
		B_h[i] = rand() % 50;
	}

	//初始化设备数据
	//标志位表示数据只读，并且从nums1_h和nums2_h复制数据
	cl_mem A_d = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * M * P, A_h, &error);
	OPENCL_CHECK_ERRORS(error);
	cl_mem B_d = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(float) * P * N, B_h, &error);
	OPENCL_CHECK_ERRORS(error);
	cl_mem C_d = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * M * N, NULL, &error);
	OPENCL_CHECK_ERRORS(error);

	clock_t start, end;
	start = clock();
	RunAsCpu(A_h, B_h, C_h);
	end = clock();
	printf("CPU运行时间：%f\n", (float)(end -start) / CLOCKS_PER_SEC);
	
	//读取kernel.cl文件内容
	FILE *fp = fopen("kernel.cl", "rb");
	fseek(fp, 0, SEEK_END);
	size_t src_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *source = (char *)malloc(src_size * sizeof(char));
	fread((void *)source, 1, src_size, fp);
	fclose(fp);

	//创建编译运行kernel函数
	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source, &src_size, &error);
	OPENCL_CHECK_ERRORS(error);
	free(source);
	source = NULL;

	//Build the program
	error = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
	if (error != CL_SUCCESS){ //用于输出kernel代码的错误
		size_t len;
		char buffer[8 * 1024];
		printf("Error: Failed to build program executable!\n");
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
		printf("%s\n", buffer);
	}
	OPENCL_CHECK_ERRORS(error);

	//show the log
	char *build_log;
	size_t log_size;
	//First call to know the proper size
	clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
	build_log = (char *)malloc((log_size + 1) * sizeof(char));
	//Second call to get the log
	clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);
	build_log[log_size] = '\0';
	printf("%s\n", build_log);
	free(build_log);
	build_log = NULL;

	//Extracting the kernel
	cl_kernel run_as_gpu_1 = clCreateKernel(program, "RunAsGpu_1", &error);
	OPENCL_CHECK_ERRORS(error);
	//设置kernel参数
	cl_int M_d = M;
	cl_int P_d = P;
	cl_int N_d = N;
	error = clSetKernelArg(run_as_gpu_1, 0, sizeof(cl_mem), &A_d);
	error |= clSetKernelArg(run_as_gpu_1, 1, sizeof(cl_mem), &B_d);
	error |= clSetKernelArg(run_as_gpu_1, 2, sizeof(int), &M_d);
	error |= clSetKernelArg(run_as_gpu_1, 3, sizeof(int), &N_d);
	error |= clSetKernelArg(run_as_gpu_1, 4, sizeof(int), &P_d);
	error |= clSetKernelArg(run_as_gpu_1, 5, sizeof(cl_mem), &C_d);
	OPENCL_CHECK_ERRORS(error);

	//启动kernel
	size_t globalws_1[2] = {M, N};
	cl_event ev;
	error = clEnqueueNDRangeKernel(queue, run_as_gpu_1, 2, NULL, globalws_1, NULL, 0, NULL, &ev);
	OPENCL_CHECK_ERRORS(error);
	clFinish(queue);

	//计算kernel执行时间
	cl_ulong startTime, endTime;
	clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &startTime, NULL);
	clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endTime, NULL);
	cl_ulong kernelExecTimeNs = endTime - startTime;
	printf("GPU_1运行时间：%8.6fms\n", kernelExecTimeNs * 1e-6);

	//取得kernel返回值并比较结果
	float * gpu_C_1 = (float *)malloc(M * N * sizeof(float));
	clEnqueueReadBuffer(queue, C_d, CL_TRUE, 0, M * N * sizeof(float), gpu_C_1, 0, NULL, NULL);
	assert(memcmp(C_h, gpu_C_1, M * N * sizeof(float)) == 0);

	//Extracting the kernel
	cl_kernel run_as_gpu_2 = clCreateKernel(program, "RunAsGpu_2", &error);
	OPENCL_CHECK_ERRORS(error);
	//设置kernel参数
	error = clSetKernelArg(run_as_gpu_2, 0, sizeof(cl_mem), &A_d);
	error |= clSetKernelArg(run_as_gpu_2, 1, sizeof(cl_mem), &B_d);
	error |= clSetKernelArg(run_as_gpu_2, 2, sizeof(int), &M_d);
	error |= clSetKernelArg(run_as_gpu_2, 3, sizeof(int), &N_d);
	error |= clSetKernelArg(run_as_gpu_2, 4, sizeof(int), &P_d);
	error |= clSetKernelArg(run_as_gpu_2, 5, sizeof(cl_mem), &C_d);
	OPENCL_CHECK_ERRORS(error);

	//启动kernel
	size_t globalws_2[2] = {M, N};
	error = clEnqueueNDRangeKernel(queue, run_as_gpu_2, 2, NULL, globalws_2, NULL, 0, NULL, &ev);
	OPENCL_CHECK_ERRORS(error);
	clFinish(queue);

	//计算kernel执行时间
	clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &startTime, NULL);
	clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endTime, NULL);
	kernelExecTimeNs = endTime - startTime;
	printf("GPU_2运行时间：%8.6fms\n", kernelExecTimeNs * 1e-6);

	//取得kernel返回值并比较结果
	float * gpu_C_2 = (float *)malloc(M * N * sizeof(float));
	clEnqueueReadBuffer(queue, C_d, CL_TRUE, 0, M * N * sizeof(float), gpu_C_2, 0, NULL, NULL);
	assert(memcmp(C_h, gpu_C_2, M * N * sizeof(float)) == 0);

	//Extracting the kernel
	cl_kernel run_as_gpu_3 = clCreateKernel(program, "RunAsGpu_3", &error);
	OPENCL_CHECK_ERRORS(error);
	//设置kernel参数
	error = clSetKernelArg(run_as_gpu_3, 0, sizeof(cl_mem), &A_d);
	error |= clSetKernelArg(run_as_gpu_3, 1, sizeof(cl_mem), &B_d);
	error |= clSetKernelArg(run_as_gpu_3, 2, sizeof(int), &M_d);
	error |= clSetKernelArg(run_as_gpu_3, 3, sizeof(int), &N_d);
	error |= clSetKernelArg(run_as_gpu_3, 4, sizeof(int), &P_d);
	error |= clSetKernelArg(run_as_gpu_3, 5, sizeof(cl_mem), &C_d);
	OPENCL_CHECK_ERRORS(error);

	//启动kernel
	size_t globalws_3[2] = {M, N};
	error = clEnqueueNDRangeKernel(queue, run_as_gpu_3, 2, NULL, globalws_3, NULL, 0, NULL, &ev);
	OPENCL_CHECK_ERRORS(error);
	clFinish(queue);

	//计算kernel执行时间
	clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &startTime, NULL);
	clGetEventProfilingInfo(ev, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endTime, NULL);
	kernelExecTimeNs = endTime - startTime;
	printf("GPU_3运行时间：%8.6fms\n", kernelExecTimeNs * 1e-6);

	//取得kernel返回值并比较结果
	float * gpu_C_3 = (float *)malloc(M * N * sizeof(float));
	clEnqueueReadBuffer(queue, C_d, CL_TRUE, 0, M * N * sizeof(float), gpu_C_3, 0, NULL, NULL);
	assert(memcmp(C_h, gpu_C_3, M * N * sizeof(float)) == 0);

	free(A_h);
	A_h = NULL;
	free(B_h);
	B_h = NULL;
	free(C_h);
	C_h = NULL;
	free(A_h);
	A_h = NULL;
	free(B_h);
	B_h = NULL;
	free(C_h);
	C_h = NULL;
	free(gpu_C_1);
	gpu_C_1 = NULL;
	free(gpu_C_2);
	gpu_C_2 = NULL;
	free(platforms);
	platforms = NULL;
	error = clReleaseKernel(run_as_gpu_1);
	error |= clReleaseKernel(run_as_gpu_2);
	error |= clReleaseKernel(run_as_gpu_3);
	error |= clReleaseCommandQueue(queue);
	error |= clReleaseContext(context);
	error |= clReleaseMemObject(A_d);
	error |= clReleaseMemObject(B_d);
	error |= clReleaseMemObject(C_d);
	OPENCL_CHECK_ERRORS(error);

	return 0;
}
