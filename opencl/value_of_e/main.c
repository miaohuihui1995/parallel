#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>

#define OPENCL_CHECK_ERRORS(error)                                       \
	if(error != CL_SUCCESS){                                         \
		printf("errorcode: %d, at line %d\n", error, __LINE__);  \
		exit(1);                                                 \
	}                                                                

int main(int argc, char **argv){

	int MaxItem = 100;
	int StepNum = 1000000;
	cl_int error = 0;
        cl_context context;
        cl_command_queue queue;
        cl_device_id device;

        cl_uint num_of_platforms = 0;
        error = clGetPlatformIDs(0, 0, &num_of_platforms);
        OPENCL_CHECK_ERRORS(error);
        printf("可用平台数：%d\n", num_of_platforms);

        cl_platform_id *platforms = (cl_platform_id *)malloc(num_of_platforms * sizeof(cl_uint));
        error = clGetPlatformIDs(num_of_platforms, platforms, 0);
        OPENCL_CHECK_ERRORS(error);

        cl_uint selected_platform_index = num_of_platforms;
        for(cl_uint i = 0; i < num_of_platforms; i++){
                size_t platform_name_length = 0;
                error = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, 0, 0, &platform_name_length);
                OPENCL_CHECK_ERRORS(error);

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

        cl_platform_id platform = platforms[selected_platform_index];
        error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
        OPENCL_CHECK_ERRORS(error);

        context = clCreateContext(0, 1, &device, NULL, NULL, &error);
        OPENCL_CHECK_ERRORS(error);

        queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &error);
        OPENCL_CHECK_ERRORS(error);

        FILE *fp = fopen("kernel.cl", "rb");
        fseek(fp, 0, SEEK_END);
        size_t src_size = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        char *source = (char *)malloc(src_size * sizeof(char));
        fread((void *)source, 1, src_size, fp);
        fclose(fp);

        cl_program program = clCreateProgramWithSource(context, 1, (const char **)&source, &src_size, &error);
        OPENCL_CHECK_ERRORS(error);
        free(source);
        source = NULL;

        error = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
        if (error != CL_SUCCESS){
                size_t len;
                char buffer[8 * 1024];
                printf("Error: Failed to build program executable!\n");
                clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
                printf("%s\n", buffer);
        }
        OPENCL_CHECK_ERRORS(error);

        char *build_log;
        size_t log_size;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        build_log = (char *)malloc((log_size + 1) * sizeof(char));
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, build_log, NULL);
        build_log[log_size] = '\0';
        printf("%s\n", build_log);
        free(build_log);
        build_log = NULL;

	float *C = (float *)malloc(sizeof(float) * MaxItem);
	cl_mem bufferC = clCreateBuffer(context, CL_MEM_WRITE_ONLY, MaxItem * sizeof(float), NULL, &error);
        OPENCL_CHECK_ERRORS(error);

	size_t globalws = MaxItem;
	cl_event ev[2];
	double e = 0;
	cl_ulong startTime, endTime, kernelExecTimeNs;

	cl_kernel method1 = clCreateKernel(program, "getValueE_1", &error);
        OPENCL_CHECK_ERRORS(error);
	error = clSetKernelArg(method1, 0, sizeof(cl_mem), &bufferC);
	error |= clSetKernelArg(method1, 1, sizeof(int), &StepNum);
	error |= clSetKernelArg(method1, 2, sizeof(int), &MaxItem);
        OPENCL_CHECK_ERRORS(error);

	error = clEnqueueNDRangeKernel(queue, method1, 1, NULL, &globalws, NULL, 0, NULL, &ev[0]);
        OPENCL_CHECK_ERRORS(error);
	error = clWaitForEvents(1, &ev[0]); //时间同步 
        OPENCL_CHECK_ERRORS(error);

        clGetEventProfilingInfo(ev[0], CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &startTime, NULL);
        clGetEventProfilingInfo(ev[0], CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endTime, NULL);
        kernelExecTimeNs = endTime - startTime;
        printf("method1运行时间：%8.6fms\n", kernelExecTimeNs * 1e-6);


	error = clEnqueueReadBuffer(queue, bufferC, CL_TRUE, 0, MaxItem * sizeof(float), C, 0, NULL, &ev[1]);
        OPENCL_CHECK_ERRORS(error);
	error = clWaitForEvents(1, &ev[1]); 
        OPENCL_CHECK_ERRORS(error);

	for(int i = 0; i < MaxItem; i++){
		e += C[i];
	}
	printf("method1 e = %1.22f\n", e + 1);

	cl_kernel method2 = clCreateKernel(program, "getValueE_2", &error);
        OPENCL_CHECK_ERRORS(error);
	error = clSetKernelArg(method2, 0, sizeof(cl_mem), &bufferC);
	error |= clSetKernelArg(method2, 1, sizeof(int), &StepNum);
	error |= clSetKernelArg(method2, 2, sizeof(int), &MaxItem);
        OPENCL_CHECK_ERRORS(error);

	error = clEnqueueNDRangeKernel(queue, method2, 1, NULL, &globalws, NULL, 0, NULL, &ev[0]);
        OPENCL_CHECK_ERRORS(error);
	error = clWaitForEvents(1, &ev[0]); //时间同步 
        OPENCL_CHECK_ERRORS(error);

        clGetEventProfilingInfo(ev[0], CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &startTime, NULL);
        clGetEventProfilingInfo(ev[0], CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &endTime, NULL);
        kernelExecTimeNs = endTime - startTime;
        printf("method2运行时间：%8.6fms\n", kernelExecTimeNs * 1e-6);

	error = clEnqueueReadBuffer(queue, bufferC, CL_TRUE, 0, MaxItem * sizeof(float), C, 0, NULL, &ev[1]);
        OPENCL_CHECK_ERRORS(error);
	error = clWaitForEvents(1, &ev[1]); 
        OPENCL_CHECK_ERRORS(error);

	e = 0.0;
	for(int i = 0; i < MaxItem; i++){
		e += C[i];
	}
	printf("method2 e = %1.22f\n", e + 1);


	error = clReleaseEvent(ev[0]);
	error |= clReleaseEvent(ev[1]);
        OPENCL_CHECK_ERRORS(error);

	free(C);
	C = NULL;
        error = clReleaseKernel(method1);
        error = clReleaseKernel(method2);
        error |= clReleaseCommandQueue(queue);
        error |= clReleaseContext(context);
        error |= clReleaseMemObject(bufferC);
        OPENCL_CHECK_ERRORS(error);

	return 0;
}
