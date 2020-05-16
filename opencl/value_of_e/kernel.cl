//多项式分段kernel
__kernel void getValueE_1(__global float *result, int StepNum, int MaxItem){
	int id = get_global_id(0);
	float fact = 1;
	float e = 0;
	for(int i = id + 1; i <= StepNum; i += MaxItem){
		for(int j = 0; j < MaxItem && j < i; j++){
			fact *= (i - j);
		}
		e += (1.0 / fact);
	}
	result[id] = e;
}

//提取公因式kernel
__kernel void getValueE_2(__global float *result, int StepNum, int MaxItem){
	int id = get_global_id(0);
	float start, end, res;
	int offset = StepNum / MaxItem;
	start = id * offset + 1;
	end = (id + 1) * offset + 1;
	res = 0;
	float fact = 1;
	for(int i = start; i < end; i++){
		fact *= i;
		res += (1.0 / fact);
	}
	result[id * 2] = res;
	result[id * 2 + 1] = 1 / fact;
	barrier(CLK_LOCAL_MEM_FENCE);
}
