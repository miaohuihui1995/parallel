//#pragma OPENCL EXTENSION cl_khr_fp64: enable
//这里OpenCL扩展 cl_khr_fp64已经受内部支持了，所用不需要再写以上的代码
//运行会生成日志文件ocl_log.txt
#pragma warning(disable: 4996)

//x = ax + y
__kernel void kernelDaxpy(int n, int ibeg, __global double *x, __global double *y, double a){
	const int gid = get_global_id(0); if(gid >= n) return;
	const int row = gid + ibeg;
	x[row] = a * x[row] + y[row];
}
