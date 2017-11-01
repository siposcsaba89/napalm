#include <napalm/napalm.h>
#include <vector>
#include <cassert>
#include "napalm/gen/programs/simple.h"
int main()
{

    std::string platforms[] = { "CUDA", "OpenCL" };
    //std::string platforms[] = { "OpenCL" };

    for (auto & pl : platforms)
    {
        napalm::PlatformAndDeviceInfo * platforms_and_devices = napalm::getPlatformAndDeviceInfo(pl.c_str());
        //Test copy
        napalm::PlatformAndDeviceInfo other = *platforms_and_devices;
        for (int32_t i = 0; i < other.num_platforms; ++i)
        {
            std::cout << other.platforms[i] << std::endl;
            for (int32_t j = 0; j < other.num_devices[i]; ++j)
            {
                std::cout << "\t" << other.device_names[i][j] << std::endl;
            }
        }
        delete platforms_and_devices;
        napalm::Context * cl_ctx = napalm::createContext(pl.c_str(), 1, 0, 5);

        //test buffer read write
        napalm::Buffer * d_buff = cl_ctx->createBuffer(256);
        napalm::Buffer * d_buff_out = cl_ctx->createBuffer(256);
        std::vector<uint8_t> buff(256, 1);
        d_buff->write(buff.data());
        std::vector<uint8_t> buff2(256, 3);
        d_buff->read(buff2.data());

        for (size_t i = 0; i < buff2.size(); ++i)
        {
            assert(buff2[i] == buff[i] && "Memory read write error!");
        }


        //test image read write
        napalm::Img * d_img = cl_ctx->createImg(napalm::ImgFormat(), napalm::ImgRegion(256, 256));
        std::vector<uint8_t> img_buff(256 * 256, 1);
        d_img->write(img_buff.data());
        std::vector<uint8_t> img_buff2(256 * 256, 3);
        d_img->read(img_buff2.data());

        for (size_t i = 0; i < img_buff2.size(); ++i)
        {
            assert(img_buff2[i] == img_buff[i] && "Memory read write error!");
        }

        //test image3d read write
        napalm::Img * d_img3d = cl_ctx->createImg(napalm::ImgFormat(), napalm::ImgRegion(256, 256, 256));
        std::vector<uint8_t> img_buff3d(256 * 256 * 256, 1);
        d_img3d->write(img_buff3d.data());
        std::vector<uint8_t> img_buff2_3d(256 * 256 * 256, 3);
        d_img3d->read(img_buff2_3d.data());

        for (size_t i = 0; i < img_buff2_3d.size(); ++i)
        {
            assert(img_buff2_3d[i] == img_buff3d[i] && "Memory read write error!");
        }

        int multiplier = 3;

        if (pl == "OpenCL")
        {
            //test simple cl program
            napalm::ProgramData test_prog =
            {
                napalm::ProgramData::DATA_TYPE_SOURCE_DATA,
                "kernel void test_kernel(global unsigned char * src, global unsigned char * dst, int multiplier)"
                "{dst[get_global_id(0)] = src[get_global_id(0)] * multiplier;}",
                0
            };
            napalm::Program * prog = cl_ctx->createProgram(test_prog);
            napalm::Kernel & test_kernel = prog->getKernel("test_kernel");
            test_kernel(1, napalm::ImgRegion(16), napalm::ImgRegion(16),
                *d_buff, *d_buff_out, multiplier);
            cl_ctx->finish(1);
            d_buff_out->read(buff2.data());
            for (size_t i = 0; i < buff2.size(); ++i)
            {
                assert(buff2[i] == multiplier * buff[i] && "Kernel execution error");
            }

            std::cout << prog->getBinary().data << std::endl;
            std::cout << prog->getBinary().binary_size << std::endl;
            delete prog;

        }
        else if (pl == "CUDA")
        {
            //test simple cl program
            napalm::ProgramData test_prog =
            {
                napalm::ProgramData::DATA_TYPE_SOURCE_DATA,
                "extern \"C\" {__global__ void test_kernel(unsigned char * src, unsigned char * dst, int multiplier)"
                "{ dst[threadIdx.x + blockIdx.x * blockDim.x] = src[threadIdx.x + blockIdx.x * blockDim.x] * multiplier;}}",
                0
            };
            napalm::Program * prog = cl_ctx->createProgram(test_prog);
            napalm::Kernel & test_kernel = prog->getKernel("test_kernel");
            test_kernel(1, napalm::ImgRegion(16), napalm::ImgRegion(16),
                *d_buff, *d_buff_out, multiplier);
            cl_ctx->finish(1);
            d_buff_out->read(buff2.data());
            for (size_t i = 0; i < buff2.size(); ++i)
            {
                assert(buff2[i] == multiplier * buff[i] && "Kernel execution error");
            }

            std::cout << prog->getBinary().data << std::endl;
            std::cout << prog->getBinary().binary_size << std::endl;
            delete prog;

        }

        napalm::ProgramStore * pr_store = napalm::ProgramStore::create(cl_ctx);
        napalm::gen::simple f_temp(*pr_store);
        multiplier = 16;
        f_temp("test_kernel")(1, napalm::ImgRegion(16), napalm::ImgRegion(16),
            *d_buff, *d_buff_out, multiplier);
        cl_ctx->finish(1);
        d_buff_out->read(buff2.data());
        for (size_t i = 0; i < buff2.size(); ++i)
        {
            assert(buff2[i] == multiplier * buff[i] && "Kernel execution error");
        }

        delete d_buff;
        delete d_buff_out;
        delete d_img3d;
        delete d_img;
        napalm::destroyContext(cl_ctx);
    }
    return 0;
}