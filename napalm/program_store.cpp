#include "program_store.h"
#include <fstream>
#include "napalm.h"

namespace napalm {

    ProgramStore::ProgramStore(Context * dev_ctx) : m_dev_ctx(dev_ctx)
    {
    }

    ProgramStore * ProgramStore::create(Context * dev_ctx)
    {
        if (dev_ctx->getProgramStore() == nullptr)
        {
            ProgramStore * ret = new ProgramStore(dev_ctx);
            dev_ctx->setProgramStore(ret);
            return ret;
        }
        else
            return dev_ctx->getProgramStore();
    }

    ProgramStore::~ProgramStore()
    {
        for (auto & pr : m_program_map)
        {
            delete pr.second;
        }
    }

    Program * ProgramStore::getProgram(const std::string & name)
    {
        Program * ret = nullptr;
        auto it = m_program_map.find(name);
        if (it == m_program_map.end())
        {
            printf("Cannot find %s in program, please call addProgram befor getProgram! \n", name.c_str());
        }
        else
            ret = it->second;
        return ret;
    }

    Program * ProgramStore::addProgram(const std::string & name, const ProgramData & data)
    {
        Program * ret = nullptr;
        auto it = m_program_map.find(name);
        const ProgramData * pr_data_to_build = &data;
        ProgramData * binary = nullptr;
        if (it == m_program_map.end())
        {
            if (data.data_type == ProgramData::DATA_TYPE_SOURCE_DATA || data.data_type == ProgramData::DATA_TYPE_SOURCE_FILE_NAME)
            {
                //load binary and check timestamp
                std::ifstream ts_file(name + ".bints");
                std::string stored_ts;
                if (ts_file.is_open())
                {
                    ts_file >> stored_ts;
                    if (stored_ts == data.timestamp)
                    {
                        //load binary file and create programData
                        std::ifstream binpr(name + ".binpr", std::ios::binary);
                        if (binpr.is_open())
                        {
                            std::string pr_binary((std::istreambuf_iterator<char>(binpr)),
                                std::istreambuf_iterator<char>());
                            binary = new ProgramData();
                            char * pr_binary_data = new char[pr_binary.size()];
                            memcpy(pr_binary_data, pr_binary.data(), pr_binary.size());
                            binary->data = pr_binary_data;
                            binary->data_size = pr_binary.size();
                            binary->data_type = ProgramData::DATA_TYPE_BINARY_DATA;
                            binary->timestamp = data.timestamp;
                            pr_data_to_build = binary;
                        }
                    }
                }
            }
            Program * prog = m_dev_ctx->createProgram(*pr_data_to_build);
            bool program_loaded_succesfully = false;
            if (prog != nullptr && prog->getStatus())
            {
                program_loaded_succesfully = true;
            }
            else if (prog != nullptr && prog->getStatus() == false && binary != nullptr)
            {
                delete prog;
                prog = m_dev_ctx->createProgram(data);
                if (prog->getStatus())
                {
                    program_loaded_succesfully = true;
                    pr_data_to_build = &data;
                }
            }
            if (program_loaded_succesfully)
            {
                m_program_map[name] = prog;
                ret = prog;
                if (m_program_cache_enabled && (pr_data_to_build->data_type == ProgramData::DATA_TYPE_SOURCE_DATA ||
                    pr_data_to_build->data_type == ProgramData::DATA_TYPE_SOURCE_FILE_NAME))
                {
                    std::ofstream bints(name + ".bints");
                    bints << data.timestamp;
                    ProgramBinary pb = prog->getBinary();
                    std::ofstream program_binary(name + ".binpr", std::ios::binary);
                    program_binary.write(pb.data, pb.binary_size);
                }
            }
            if (binary)
            {
                delete[] binary->data;
                delete binary;
            }

        }
        else
            ret = it->second; // program is already on the store
        return ret;
    }

    void ProgramStore::enableProgramCache(bool cache_enabled)
    {
        m_program_cache_enabled = cache_enabled;
    }

    const Context * ProgramStore::getContext() const
    {
        return m_dev_ctx;
    }
}