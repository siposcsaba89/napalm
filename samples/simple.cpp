#include <napalm/napalm.h>
#include <vector>
#include <cassert>

int main()
{
    napalm::Context * cl_ctx = napalm::createContext("OpenCL", 2, 0, 5);

    //test buffer read write
    napalm::Buffer * d_buff = cl_ctx->createBuffer(256);
    std::vector<uint8_t> buff(256, 1);
    d_buff->write(buff.data());
    std::vector<uint8_t> buff2(256, 3);
    d_buff->read(buff2.data());

    for (size_t i = 0; i < buff2.size(); ++i)
    {
        assert( buff2[i] == buff[i] && "Memory read write error!");
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


    delete d_buff;
    delete d_img3d;
    delete d_img;
    delete cl_ctx;
    return 0;
}