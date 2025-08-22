#include "SC_FileSaver.h"
#include "..\SC_Server.h"
#include <fstream>
//#include "Library/WinAPI.h"

// ----------------------------------
void SC_ServerFileSaver::SaveFile(const std::string& path, const std::string& data) {

    std::unique_ptr<SC_ServerFileSaver> pTask = std::make_unique<SC_ServerFileSaver>();

    // Set path and Data
    pTask->m_FilePath = path;
    pTask->m_Data = data;

    // Pass the task to the executor
    gSC_Server->GetExecutor()->AddTask(std::move(pTask));
}

// ----------------------------------
void SC_ServerFileSaver::OnFileSaved(bool Status) {

    if (Status)
    {
        //gServer->Log("File %s has been Saved Successfully", (FileName.filename().empty() ? FileName.c_str() : FileName.filename().c_str()));
    }
    else
    {
        gSC_Server->LogError("Failed to Save the File %s", m_FilePath.c_str());
    }
}

// ----------------------------------
bool SC_ServerFileSaver::WriteToFile() {
   
    // Open the file with output mode
    std::ofstream pFile(m_FilePath, std::ios::out | std::ios::trunc);

    // Check if file opened successfully
    bool IsOpen = pFile.is_open();
    if (IsOpen) {
        pFile << m_Data; // Write data to file
        pFile.close();   // Close the file
    }

    return IsOpen;
}