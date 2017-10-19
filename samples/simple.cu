

extern "C"
{
    __global__ void test_kernel( const unsigned char * src, unsigned char * dst, int multiplier)
    {
        dst[threadIdx.x + blockIdx.x * blockDim.x] = src[threadIdx.x + blockIdx.x * blockDim.x] * multiplier;
    }
}