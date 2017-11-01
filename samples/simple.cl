#include <sample_inc.h>


kernel void test_kernel(global unsigned char * src, global unsigned char * dst, int multiplier)
{
    dst[get_global_id(0)] = src[get_global_id(0)] * multiplier;
} 