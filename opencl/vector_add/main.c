#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>
#include <CL/cl_platform.h>

#define OPENCL_CHECK_ERRORS(error)                                    \
	if(error != CL_SUCCESS){                                      \
    		printf("error: %d, at line %d\n", error, __LINE__);   \
		exit(1);                                              \
	}

static int Align(int i, int a){
	int ret = i;
	if(ret % a > 0){
		ret += (a - ret % a);
	}
	return ret;
}

float *Vector_Add_Cpu(float *a, float *b, float *c, const int n){
	for(int i = 0; i < n; i++){
		c[i] = a[i] + b[i];
	}
	return c;
}

int main(){

	cl_int error = 0;
	cl_platform_id firstplatform;
	cl_uint num_platforms;
	cl_context context;
	cl_command_queue queue;
	cl_device_id device;

	error = clGetPlatformIDs(1, &firstplatform, &num_platforms);
	OPENCL_CHECK_ERRORS(error);
	error = clGetDeviceIDs(firstplatform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	OPENCL_CHECK_ERRORS(error);
	context = clCreateContext(0, 1, &device, NULL, NULL, &error);
	OPENCL_CHECK_ERRORS(error);
	queue = clCreateCommandQueue(context, device, 0, &error);
	OPENCL_CHECK_ERRORS(error);

	const int size = 123456;
	float *src_a_h = (float *)malloc(sizeof(float) *size);
	float *src_b_h = (float *)malloc(sizeof(float) *size);
	float *res_h = (float *)malloc(sizeof(float) *size);
	for(int i = 0; i < size; i++){
		src_a_h[i] = src_b_h[i] = (float)i;
	}
	const int mem_size = sizeof(float) * size;
	cl_mem src_a_d = clCreateBuffer(context, CL_MEM_READ_WRITE, mem_size, NULL, &error);
	OPENCL_CHECK_ERRORS(error);
	cl_mem src_b_d = clCreateBuffer(context, CL_MEM_READ_WRITE, mem_size, NULL, &error);
	OPENCL_CHECK_ERRORS(error);
	cl_mem res_d = clCreateBuffer(context, CL_MEM_WRITE_ONLY, mem_size, NULL, &error);
	OPENCL_CHECK_ERRORS(error);
	/*CL_MEM_READ_WRITE
	CL_MEM_WRITE_ONLY
	CL_MEM_READ_ONLY
	CL_MEM_USE_HOST_PTR
	CL_MEM_ALLOC_HOST_PTR
	CL_MEM_COPY_HOST_PTR – 从 host_ptr处拷贝数据*/
	clEnqueueWriteBuffer(queue, src_a_d, CL_TRUE, 0, mem_size, src_a_h, 0, NULL, NULL);
	OPENCL_CHECK_ERRORS(error);
	clEnqueueWriteBuffer(queue, src_b_d, CL_TRUE, 0, mem_size, src_b_h, 0, NULL, NULL);
	OPENCL_CHECK_ERRORS(error);

	const char *path = "kernel.cl";
	const char *cDefines = "#define TEST 1\n";
	char *source = NULL;
	FILE *f = fopen(path, "rb");
        fseek(f, 0, SEEK_END);
        size_t fileSize = ftell(f);
        rewind(f);
	int codeSize = fileSize + strlen(cDefines);
        source = (char *)malloc((codeSize + 1) * sizeof(char));
        memcpy(source, cDefines, strlen(cDefines));
	size_t nd = fread(source + strlen(cDefines), 1, fileSize, f);
        if(nd != fileSize){
                printf("Failed to read program file!!\n");
	}       
        source[codeSize] = 0;
	size_t src_size = strlen(source);

	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source, &src_size, &error);
	OPENCL_CHECK_ERRORS(error);
	error = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
	OPENCL_CHECK_ERRORS(error);
	cl_kernel Vector_Add_Gpu = clCreateKernel(program, "vector_add_gpu", &error);
	//注意：这里Vector_Add_Gpu为host文件内的变量，而vector_add_gpu为cl文件内kernel函数名称，不搞写错!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	OPENCL_CHECK_ERRORS(error);

	clSetKernelArg(Vector_Add_Gpu, 0, sizeof(cl_mem), &src_a_d);
	clSetKernelArg(Vector_Add_Gpu, 1, sizeof(cl_mem), &src_b_d);
	clSetKernelArg(Vector_Add_Gpu, 2, sizeof(cl_mem), &res_d);
	clSetKernelArg(Vector_Add_Gpu, 3, sizeof(const int), &size);
	const size_t local_ws = 128; //Number of work-items per work-group
	const size_t global_ws = Align(size, local_ws); //Total number of work-items
	error = clEnqueueNDRangeKernel(queue, Vector_Add_Gpu, 1, NULL, &global_ws, &local_ws, 0, NULL, NULL);
	OPENCL_CHECK_ERRORS(error);
	clFlush(queue);
        clFinish(queue);

	float *check = (float *)malloc(sizeof(float) * size);
	error = clEnqueueReadBuffer(queue, res_d, CL_TRUE, 0, mem_size, check, 0, NULL, NULL);
	OPENCL_CHECK_ERRORS(error);

	res_h = Vector_Add_Cpu(src_a_h, src_b_h, res_h, size);
	float sum = 0;
	for(int i = 0; i < size; i++){
		sum += fabs(res_h[i] - check[i]);
	}
	printf("ERR between Vector_Add_Cpu and Vector_Add_Gpu is %f\n", sum);

	free(src_a_h);
	src_a_h = NULL;
	free(src_b_h);
	src_b_h = NULL;
	free(res_h);
	res_h = NULL;
	free(check);
	check = NULL;
	error = clReleaseKernel(Vector_Add_Gpu);
	OPENCL_CHECK_ERRORS(error);
	error = clReleaseCommandQueue(queue);
	OPENCL_CHECK_ERRORS(error);
	error = clReleaseContext(context);
	OPENCL_CHECK_ERRORS(error);
	error = clReleaseMemObject(src_a_d);
	OPENCL_CHECK_ERRORS(error);
	error = clReleaseMemObject(src_b_d);
	OPENCL_CHECK_ERRORS(error);
	error = clReleaseMemObject(res_d);
	OPENCL_CHECK_ERRORS(error);

	return 0;
}
