#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>
#include <CL/cl_platform.h>


/*======================================================================
Utility functions
======================================================================*/
#define LINE_SEPARATOR "------------------------------------------------------------------\n"
#define crash(...) exit(Crash(__VA_ARGS__)) //via exit define so static analyzer its an exit point
static int Crash(const char *fmt, ...){ //crash!!!
	//这里用到可变参数
	va_list ap;
	fprintf(stderr, "\nEpic fail: \n");
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	fflush(stderr);
	return 0;
}

static void createKernel(cl_kernel &k, cl_program &p, const char *kname){
	//这里cl_kernel &k和cl_program &p是应用了引用传递，这是C++的特性，必须要用g++编译
	int err = 0;
	k = clCreateKernel(p, kname, &err);
	if(err){
		crash("createKernel: fail for %s with code %d\n", kname, err);
	}
	printf("%-20s created\n", kname);
}

static int Align(int i, int a){
	int ret = i;
	if(ret % a > 0){
		ret += (a - ret % a);
	}
	return ret;
}


/*======================================================================
Main
======================================================================*/
int main(int argc, char **argv){
	//input parameters
	cl_int devID = 0; //device number
	const char *platformName = "Inter(R) OpenCL";
	const int N = 100000000; //test vector size

	//platforms
	cl_uint platformCount = 0; //number of platforms
	cl_platform_id *platformList = NULL; //list of platforms
	cl_int platformid = 0; //number of the selected platform

	//devices
	int deviceCount = 0; //number of devices
	cl_device_id *deviceList = NULL; //device list

	cl_context cxGPUContext; //OpenCL context
	cl_command_queue cqCommandQueue; //OpenCL command queue
	cl_command_queue StLdCommandQueue; //OpenCL command queue
	cl_program cpProgram; //OpenCL program


/*======================================================================
Platforms
======================================================================*/
	printf(LINE_SEPARATOR);
	printf("OpenCL initializing platforms and devices\n");
	printf(LINE_SEPARATOR);

	//getting number of platforms
	cl_int ciErrNum = clGetPlatformIDs(0, 0, &platformCount);
	if(ciErrNum != CL_SUCCESS){
		crash("Error: (clGetPlatformIDs).Error N: %d\n", ciErrNum);
	}
	else{
		printf("clGetPlatformIDs: platformCount ok \n");
	}

	//getting list of platforms
	platformList = (cl_platform_id *)malloc(platformCount * sizeof(cl_uint)); //???
	ciErrNum = clGetPlatformIDs(platformCount, platformList, 0);
	if(ciErrNum != CL_SUCCESS){
		crash("Error: (clGetPlatformIDs).Error N: %d\n", ciErrNum);
	}
	else{
		printf("clGetPlatformIDs list ok \n");
	}

	size_t paramSize = 1024;
	char platformNameBuf[1024];

	//looking for the platform with name platformName
	for(unsigned int i = 0; i < platformCount; i++){
		clGetPlatformInfo(platformList[i], CL_PLATFORM_NAME, paramSize, platformNameBuf, 0);
		printf("Available platform: %d: %s\n", i, platformNameBuf);
		if(!strcmp(platformName, platformNameBuf)){
			platformid = i;
		}
	}
	if(platformid < 0){
		crash("Can't find platform neede \n");
	}

	printf("Platform %d selected\n", platformid);
	printf("GPU selected\n");

	//getting number of devices
	ciErrNum = clGetDeviceIDs(platformList[platformid], CL_DEVICE_TYPE_ALL, 0, NULL, (cl_uint *) &deviceCount); 
	if(ciErrNum != CL_SUCCESS){
		crash("Error: (clGetDeviceIDs).Error N: %d\n", ciErrNum);
	}
	deviceList = (cl_device_id *)malloc(deviceCount * sizeof(int)); //???
	printf("%d devices found\n", deviceCount);

	//getting list devices
	if(deviceList != NULL){
		ciErrNum = clGetDeviceIDs(platformList[platformid], CL_DEVICE_TYPE_ALL, (cl_uint)deviceCount, deviceList, NULL);
	}
	else{
		crash("No devices available!\n");
	}
	if(ciErrNum != CL_SUCCESS){
		crash("Error: (clGetDeviceIDs).Error N: %d\n", ciErrNum);
	}

	//print devices
	for(int i = 0; i < deviceCount; i++){
		clGetDeviceInfo(deviceList[i], CL_DEVICE_NAME, paramSize, platformNameBuf, 0);
		printf("DEV: %d: %s \n", i, platformName);
	}


/*======================================================================
Context
======================================================================*/
	cl_uint num_compute_units = 0;
	ciErrNum = 0;
	if(devID < 0 || devID >= deviceCount){
		crash("OpenCL_InitContext: Can't select device needed!\n");
	}
	printf("\nOpenCL init for device %d: \n", devID);
	//getting device's info
	clGetDeviceInfo(deviceList[devID], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(num_compute_units), &num_compute_units, NULL);
	printf("device has %d compute units\n", num_compute_units);
	//creating context
	cxGPUContext = clCreateContext(NULL, 1, &deviceList[devID], 0, 0, &ciErrNum);
	//creating command-queues within the context
	//queue for execution
	cqCommandQueue = clCreateCommandQueue(cxGPUContext, deviceList[devID], 0, &ciErrNum);
	//queue for data transfer
	StLdCommandQueue = clCreateCommandQueue(cxGPUContext, deviceList[devID], 0, &ciErrNum);
	printf("GPU context and queues created\n");


/*======================================================================
Program
======================================================================*/
	//cpProgram	- OpenCL program object to fill
	//cPathAndName	- path to source code file
	//cDefines	- text with definitions to prepend source code file

	int info = 0; //flag to print program source code
	ciErrNum = 0;
	const char *cPathAndName = "kernel.cl"; //program source file name
	printf("\nOpencL loading program %s... \n", cPathAndName);
	const char *cDefines = "#define TEST 1\n"; //text with definition to prepend source code file
	//reading OpenCL program from source file to cSourceCL
	char *cSourceCL = NULL; //buffer with source
	FILE *f = fopen(cPathAndName, "rb");
	if(!f){
		crash("Can't open program file %s!\n", cPathAndName);
	}
	fseek(f, 0, SEEK_END);
	size_t fileSize = ftell(f);
	rewind(f);
	if(fileSize == 0){
		crash("Empty program file %s!\n", cPathAndName);
	}
	int codeSize = fileSize + strlen(cDefines);
	cSourceCL = (char *)malloc((codeSize + 1) * sizeof(char)); //???
	memcpy(cSourceCL, cDefines, strlen(cDefines));
	size_t nd = fread(cSourceCL + strlen(cDefines), 1, fileSize, f);
	if(nd != fileSize){
		crash("Failed to read program file %s!!\n", cPathAndName);
	}
	cSourceCL[codeSize] = 0;
	if(cSourceCL == NULL){
		crash("Can't get program source from file %s!\n", cPathAndName);
	}

	if(info){
		printf(LINE_SEPARATOR);
		printf("%s", cSourceCL);
		printf(LINE_SEPARATOR);
	}

	size_t szKernelLength = strlen(cSourceCL);

	//create the program
	printf("clCreateProgramWithSource...");
	cpProgram = clCreateProgramWithSource(cxGPUContext, 1, (const char **)&cSourceCL, &szKernelLength, &ciErrNum);
	if(ciErrNum != CL_SUCCESS){
		crash("Loading source into cl_program (clCreateProgramWithSource).Error N: %d\n", ciErrNum);
	}
	else{
		printf("done.\n");
	}

	//build the program
	printf("cBuidProgram...");
	cl_int ciErrNumB = clBuildProgram(cpProgram, 0, NULL, "-cl-mad-enable", NULL, NULL);
	printf("done.\n");

	int LOG_S = 0;
	ciErrNum = clGetProgramBuildInfo(cpProgram, deviceList[devID], CL_PROGRAM_BUILD_LOG, 0, NULL, (size_t*)&LOG_S);
	if(ciErrNum != CL_SUCCESS){
		printf("clLoadProgram: Error: (clGetProgramBuildInfo).Error N: %d\n", ciErrNum);
	}

	if(LOG_S > 8){ //print kernel compilation error
		char *programLog = (char *)malloc(LOG_S * sizeof(char)); //???
		ciErrNum = clGetProgramBuildInfo(cpProgram, deviceList[devID], CL_PROGRAM_BUILD_LOG, LOG_S, programLog, 0);
		if(ciErrNum != CL_SUCCESS){
			printf("Error: (clGetProgramBuildInfo).Error N: %d\n", ciErrNum);
		}
		printf(LINE_SEPARATOR);
		printf("%s\n", programLog);
		printf(LINE_SEPARATOR);

		FILE *file = fopen("ocl_log.txt", "wt");
		if(file){
			fprintf(file, "%s", programLog);
			//cl_uint platformCount = 0; //number of platforms
			//cl_platform_id *platformList = NULL; //list of platforms
			fclose(file);
		}
		free(programLog);
		programLog = NULL;
	}

	if(ciErrNum != CL_SUCCESS){
		crash("oclLoadProgram: Building Binary into cl_program (clBuidingProgram).Error N: %d\n", ciErrNum);
	}
	if(ciErrNumB != CL_SUCCESS){
		crash("oclLoadProgram: Building Binary into cl_program (clBuidingProgram).Error (B) N: %d\n", ciErrNumB);
	}

	if(cSourceCL){
		free(cSourceCL);
		cSourceCL = NULL;
	}

	
/*======================================================================
Kernels
======================================================================*/
	//creating kernels from the program
	printf("Creating kernels...\n");
	cl_kernel kernelDaxpy; //x = ax + y
	createKernel(kernelDaxpy, cpProgram, "kernelDaxpy");


/*======================================================================
Buffers
======================================================================*/
	printf("Creating OpenCL buffers... \n");
	cl_mem clX = clCreateBuffer(cxGPUContext, CL_MEM_READ_WRITE, N * sizeof(double), NULL, &ciErrNum);
	cl_mem clY = clCreateBuffer(cxGPUContext, CL_MEM_READ_WRITE, N * sizeof(double), NULL, &ciErrNum);
	printf("Init part done.\n");
	printf(LINE_SEPARATOR);


/*======================================================================
TEST EXECUTION
======================================================================*/
	//host data
	double *X = (double *)malloc(sizeof(double) * N); //???
	double *Y = (double *)malloc(sizeof(double) * N); //???
	double a = 1.234;

	//filling test vectors
	for(int i = 0; i < N; i++){
		X[i] = (double)(i % 123) * (i % 456);
		Y[i] = (double)(i % 248) * (i % 134);
	}

	//copying vectors to device
	ciErrNum = clEnqueueWriteBuffer(StLdCommandQueue, clX, CL_TRUE, 0, N * sizeof(double), X, 0, NULL, NULL);
	if(ciErrNum){
		crash("EnqueueWriteBuffer: clEnqueueWriteBuffer fail for clX with error %d\n", ciErrNum);
	}
	ciErrNum = clEnqueueWriteBuffer(StLdCommandQueue, clY, CL_TRUE, 0, N * sizeof(double), Y, 0, NULL, NULL);
	if(ciErrNum){
		crash("EnqueueWriteBuffer: clEnqueueWriteBuffer fail for clY with error %d\n", ciErrNum);
	}

	size_t lws = 128; //local workgroup size
	size_t gws = Align(N, lws); //workgroup size
	//setting kernel arguments
	int ibeg = 0;
	clSetKernelArg(kernelDaxpy, 0, sizeof(int), &N);
	clSetKernelArg(kernelDaxpy, 1, sizeof(int), &ibeg);
	clSetKernelArg(kernelDaxpy, 2, sizeof(cl_mem), &clX);
	clSetKernelArg(kernelDaxpy, 3, sizeof(cl_mem), &clY);
	clSetKernelArg(kernelDaxpy, 4, sizeof(double), &a);

	//sending kernel to execution queue
	ciErrNum = clEnqueueNDRangeKernel(cqCommandQueue, kernelDaxpy, 1, NULL, &gws, &lws, 0, NULL, NULL);
	if(ciErrNum){
		crash("oclDaxpy: clEnqueueNDRangeKernel fail for kernelDaxpy with code %d\n", ciErrNum);
	}
	clFlush(cqCommandQueue);
	clFinish(cqCommandQueue); //waiting it to finish

	//doing same operation on host
	for(int i = 0; i < N; i++){
		X[i] = a * X[i] + Y[i];
	}

	//reading data from device to host
	double *R = (double *)malloc(sizeof(double) * N); //???
	ciErrNum = clEnqueueReadBuffer(StLdCommandQueue, clX, CL_TRUE, ibeg, N * sizeof(double), R, 0, NULL, NULL);
	if(ciErrNum){
		crash("EnqueueReadBuffer: clEnqueueReadBuffer fail for clX with error %d\n", ciErrNum);
	}

	//checking results
	double sum = 0;
	for(int i = 0; i < N; i++){
		sum += fabs(R[i] - X[i]);
	}
	printf("\n Test kernels execution done. \n ERR = %g\n", sum);
	free(R);
	R = NULL;
	free(X);
	X = NULL;
	free(Y);
	Y = NULL;

	//releasing buffers
	ciErrNum = clReleaseMemObject(clX);
	if(ciErrNum){
		crash("ReleaseBuffer: clReleaseMemObject fail for clX with error %d\n", ciErrNum);
	}
	ciErrNum = clReleaseMemObject(clY);
	if(ciErrNum){
		crash("ReleaseBuffer: clReleaseMemObject fail for clY with error %d\n", ciErrNum);
	}

	return 0;
}
