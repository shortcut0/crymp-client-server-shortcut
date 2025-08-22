#pragma once

#ifndef __SERVER_FILESAVER__
#define __SERVER_FILESAVER__

#include "CryMP/Common/Executor.h"

// =================================
class SC_ServerFileSaver : public IExecutorTask {
public:

    bool m_Status = false;

    SC_ServerFileSaver() {}
    ~SC_ServerFileSaver() {}

    void OnFileSaved(bool Status);
    void SaveFile(const std::string& path, const std::string& data);

    // Executor:
    void Execute() override
    {
        this->m_Status = WriteToFile();
    }

    void Callback() override
    {
        OnFileSaved(this->m_Status);
    }

private:
    std::string m_Data;
    std::string m_FilePath;

    bool WriteToFile();

};

#endif // !__SERVER_FILESAVER__