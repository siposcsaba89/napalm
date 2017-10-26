#include "napalm/opencl/cl_program.h"
#include "cl_utils.h"
#include <assert.h>

namespace napalm
{
    namespace cl
    {
        CLProgram::CLProgram(const OpenCLContext * ctx, const ProgramData & data, const char * compiler_options):
            m_ctx(ctx)
        {
            napalm::ProgramData::DataType dt = data.data_type;
            switch (dt)
            {
            case napalm::ProgramData::DATA_TYPE_BINARY_DATA:
                m_program_status = createProgramWithBinaryData(data);
                break;
            case napalm::ProgramData::DATA_TYPE_SOURCE_DATA:
                m_program_status = createProgramWithSourceData(data);
                break;
            case napalm::ProgramData::DATA_TYPE_BINARY_FILE_NAME:
                m_program_status = createProgramWithBinaryFile(data);
                break;
            case napalm::ProgramData::DATA_TYPE_SOURCE_FILE_NAME:
                m_program_status = createProgramWithSourceFile(data);
                break;
            default:
                break;
            }
            cl_int err = CL_SUCCESS;
            if (m_program_status)
            {
                cl_device_id dev_id = m_ctx->getCLDevice();
                err = clBuildProgram(m_program,
                    1,
                    &dev_id,
                    compiler_options,
                    nullptr,
                    nullptr
                );
            }
            if (err != CL_SUCCESS)
                m_program_status = false;

            if (err != CL_SUCCESS && (dt == napalm::ProgramData::DATA_TYPE_SOURCE_FILE_NAME || 
                dt == napalm::ProgramData::DATA_TYPE_SOURCE_DATA))
            {
                std::string m_build_log;
                size_t log_size = 0;
                clGetProgramBuildInfo(m_program, m_ctx->getCLDevice(), CL_PROGRAM_BUILD_LOG, 0, nullptr, &log_size);
                m_build_log.resize(log_size);
                clGetProgramBuildInfo(m_program, m_ctx->getCLDevice(), CL_PROGRAM_BUILD_LOG, log_size, &m_build_log[0], nullptr);
                std::cout << m_build_log << std::endl;
                handleError(err, "Program build failed");
            }
        }
        napalm::Kernel & napalm::cl::CLProgram::getKernel(const char * kernel_name)
        {
            CLKernel *ret = nullptr;
            auto it = m_kernels.find(kernel_name);
            if (it != m_kernels.end())
            {
                ret = it->second;
            }
            else
            {
                ret = new CLKernel(m_ctx, m_program, kernel_name);
                m_kernels[kernel_name] = ret;
            }
            assert(ret != nullptr && "Kernel has been deleted or failed to create!");
            return *ret;
        }
        bool CLProgram::getStatus() const
        {
            return m_program_status;
        }
        ProgramBinary CLProgram::getBinary()
        {
            size_t num_binaries;
            clGetProgramInfo(m_program, CL_PROGRAM_BINARY_SIZES, 0, nullptr, &num_binaries);
            std::vector<size_t> num_binarie(num_binaries);
            clGetProgramInfo(m_program, CL_PROGRAM_BINARY_SIZES, num_binaries, num_binarie.data(), nullptr);
            assert((num_binarie.size() == 1 || num_binarie[1] == 0) && "More then one binaries!");

            m_program_binary.resize(num_binarie[0]);
            char * data = &m_program_binary[0];
            clGetProgramInfo(m_program, CL_PROGRAM_BINARIES, num_binaries, &data, nullptr);
            return ProgramBinary{data, m_program_binary.size()};
        }
        CLProgram::~CLProgram()
        {
            for (auto & k : m_kernels)
            {
                delete k.second;
                k.second = nullptr;
            }
            cl_int err = clReleaseProgram(m_program);
            handleError(err, "CL Release program!");
        }
        bool CLProgram::createProgramWithSourceData(const ProgramData & data)
        {
            cl_int err = CL_SUCCESS;
            m_program = clCreateProgramWithSource(m_ctx->getCLContext(), 1, (const char **)&data.data,
                &data.data_size, &err);
            handleError(err, "OpenCL Create program with source data");
            return true;
        }
        bool CLProgram::createProgramWithSourceFile(const ProgramData & data)
        {
            std::string file_data = loadFile(data.data, false);
            if (file_data.empty())
                return false;
            cl_int err = CL_SUCCESS;
            const char * file_data_ptr = file_data.c_str();
            size_t file_data_l = file_data.size();
            m_program = clCreateProgramWithSource(m_ctx->getCLContext(), 1, &file_data_ptr,
                &file_data_l, &err);
            handleError(err, "OpenCL Create program with source data");
            return true;
        }
        bool CLProgram::createProgramWithBinaryData(const ProgramData & data)
        {
            bool ret = true;
            cl_int bin_status = CL_SUCCESS;
            cl_int err = CL_SUCCESS;
            cl_device_id dev_id = m_ctx->getCLDevice();
            m_program = clCreateProgramWithBinary(m_ctx->getCLContext(), 1, &dev_id,
                &data.data_size, (const unsigned char**)&data.data, &bin_status, &err);
            if (bin_status != CL_SUCCESS)
            {
                ret = false;
            }
            //handleError(err, "CL build Program With Binary Data!");
            return ret;

        }
        bool CLProgram::createProgramWithBinaryFile(const ProgramData & data)
        {
            bool ret = true;
            std::string file_data = loadFile(data.data, true);
            cl_int bin_status = CL_SUCCESS;
            cl_int err = CL_SUCCESS;
            cl_device_id dev_id = m_ctx->getCLDevice();
            const char * file_data_ptr = file_data.c_str();
            size_t file_data_l = file_data.size();
            m_program = clCreateProgramWithBinary(m_ctx->getCLContext(), 1, &dev_id,
                &file_data_l, (const unsigned char **)&file_data_ptr, 
                &bin_status, &err);
            if (bin_status != CL_SUCCESS)
            {
                ret = false;
            }
            handleError(err, "CL build Program With Binary Data!");
            return ret;
        }
        CLKernel::CLKernel(const OpenCLContext * ctx, cl_program program, const char * kernel_name) :
            m_ctx(ctx)
        {
            cl_int err = CL_SUCCESS;
            m_kernel = clCreateKernel(program, kernel_name, &err);
            handleError(err, "CL Create Kernel: " + std::string(kernel_name));
        }
        void CLKernel::setArg(int32_t idx, void * val, size_t sizeof_arg)
        {
            cl_int err = clSetKernelArg(m_kernel, cl_uint(idx), sizeof_arg, val);
            handleError(err, "Set kernel argumentum");
        }
        void CLKernel::setArgs(int32_t num_args, void ** argument, size_t * argument_sizes)
        {
            for (int32_t i = 0; i < num_args; ++i)
                setArg(i, argument[i], argument_sizes[i]);
        }
        void CLKernel::execute(int32_t command_queue, ImgRegion num_blocks, ImgRegion block_size)
        {
            size_t global_w_offset[3] = { 0,0,0 };
            size_t global_w_size[3] = { 
                size_t(num_blocks.x * block_size.x),
                size_t(num_blocks.y * block_size.y),
                size_t(num_blocks.z * block_size.z)};
            size_t local_w_size[3] = {
                size_t(block_size.x),
                size_t(block_size.y),
                size_t(block_size.z)};

            cl_int err = clEnqueueNDRangeKernel(m_ctx->getCQ(command_queue), m_kernel, 3, global_w_offset,
                global_w_size, local_w_size, 0, nullptr, nullptr);
        }
        CLKernel::~CLKernel()
        {
            cl_int err = clReleaseKernel(m_kernel);
            handleError(err, "OpenCL Release kernel object");
        }
    }
}
