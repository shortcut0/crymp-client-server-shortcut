// -------------------------------------
#include "SC_LuaFileSystem.h"
#include "..\SC_Server.h"
#include "CryCommon/CrySystem/ISystem.h"
#include "CryCommon/CryScriptSystem/IScriptSystem.h"

#include <vector>
#include <filesystem>
#include <string>     // For std::string

// -------------------------------------
SC_LuaFileSystem::SC_LuaFileSystem()
{

	// -----------
    m_pSS = gEnv->pScriptSystem;
    m_pSS->SetGlobalValue("GETFILES_ALL", GETFILES_ALL);
    m_pSS->SetGlobalValue("GETFILES_DIR", GETFILES_DIR);
    m_pSS->SetGlobalValue("GETFILES_FILES", GETFILES_FILES);

	// -----------
	Init(m_pSS, gEnv->pSystem);
	SetGlobalName("ServerLFS");


	// -----------
	#undef SCRIPT_REG_CLASSNAME
	#define SCRIPT_REG_CLASSNAME &SC_LuaFileSystem::

	// -----------
	SCRIPT_REG_TEMPLFUNC(FileIsFile, "file");
	SCRIPT_REG_TEMPLFUNC(FileGetName, "file");
	SCRIPT_REG_TEMPLFUNC(FileGetPath, "file");
	SCRIPT_REG_TEMPLFUNC(FileExists, "file");
	SCRIPT_REG_TEMPLFUNC(FileDelete, "file");
	SCRIPT_REG_TEMPLFUNC(FileGetSize, "file");

	// -----------
	SCRIPT_REG_TEMPLFUNC(DirGetName, "path");
	SCRIPT_REG_TEMPLFUNC(DirIsDir, "path");
	SCRIPT_REG_TEMPLFUNC(DirExists, "path");
	SCRIPT_REG_TEMPLFUNC(DirCreate, "path");
	SCRIPT_REG_TEMPLFUNC(DirGetSize, "path");
	SCRIPT_REG_TEMPLFUNC(DirGetFiles, "path, type");
}

// -------------------------------------
SC_LuaFileSystem::~SC_LuaFileSystem()
{
}

// -------------------------------------
bool SC_LuaFileSystem::Regexp(const std::string& itemName, const std::string& filter) {
    std::regex pattern(filter);
    return std::regex_match(itemName, pattern);
}

// -------------------------------------
int SC_LuaFileSystem::FileIsFile(IFunctionHandler* pH, const char* path)
{
    std::filesystem::path dirPath(path);

    try
    {
        if (std::filesystem::is_regular_file(dirPath))
        {
            return pH->EndFunction(true); // The path is a directory
        }
        else
        {
            return pH->EndFunction(false); // The path is not a directory
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        return pH->EndFunction(false, e.what()); // Return the error message if any exception occurs
    }
}

// -------------------------------------
int SC_LuaFileSystem::FileGetName(IFunctionHandler* pH, const char* path)
{
    std::filesystem::path filepath(path);
    return pH->EndFunction(filepath.filename().string().c_str());
}

// -------------------------------------
int SC_LuaFileSystem::FileGetPath(IFunctionHandler* pH, const char* path)
{
    std::filesystem::path filepath(path);
    std::string dirPath = filepath.parent_path().string();
    return pH->EndFunction(dirPath.c_str());
}



// -------------------------------------
int SC_LuaFileSystem::FileGetSize(IFunctionHandler* pH, const char* path)
{
    try {
        if (!std::filesystem::exists(path)) {
            return pH->EndFunction(0.f, "File does not exist.");
        }

        if (!std::filesystem::is_regular_file(path)) {
            return pH->EndFunction(0.f, "Path is not a regular file.");
        }

        float fileSize = std::filesystem::file_size(path);

        return pH->EndFunction(fileSize);
    }
    catch (const std::filesystem::filesystem_error& e) {
        return pH->EndFunction(0.f, e.what());
    }
    catch (const std::exception& e) {
        return pH->EndFunction(0.f, e.what());
    }

    pH->EndFunction(false, "Error");
}

// -------------------------------------
int SC_LuaFileSystem::FileExists(IFunctionHandler* pH, const char* path)
{
    try {
        if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path)) {
            return pH->EndFunction(true); // File exists
        }
        else {
            return pH->EndFunction(false, "File does not exist."); // File does not exist
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        return pH->EndFunction(false, e.what());
    }
    catch (const std::exception& e) {
        return pH->EndFunction(false, e.what());
    }

    pH->EndFunction(false, "Error");
}

// -------------------------------------
int SC_LuaFileSystem::FileDelete(IFunctionHandler* pH, const char* path)
{
    try {
        if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path)) {
            if (std::filesystem::remove(path)) {
                return pH->EndFunction(true); // File deleted successfully
            }
            else {
                return pH->EndFunction(false, "Failed to delete the file.");
            }
        }
        else {
            return pH->EndFunction(false, "File does not exist.");
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        return pH->EndFunction(false, e.what());
    }
    catch (const std::exception& e) {
        return pH->EndFunction(false, e.what());
    }

    pH->EndFunction(false, "Error");
}

// -------------------------------------
int SC_LuaFileSystem::DirGetName(IFunctionHandler* pH, const char* path)
{

    try {
        std::filesystem::path fsPath(path);
        std::string lastFolder = fsPath.parent_path().filename().string();

        // If using Lua's C API, push the result to the Lua stack
        return pH->EndFunction(lastFolder.c_str());
    }
    catch (const std::exception& e) {
        // Handle any exceptions, such as invalid paths
        return pH->EndFunction(NULL, e.what());
    }

    return pH->EndFunction();
}

// -------------------------------------
int SC_LuaFileSystem::DirIsDir(IFunctionHandler* pH, const char* path)
{
    std::filesystem::path dirPath(path);

    try
    {
        if (std::filesystem::is_directory(dirPath))
        {
            return pH->EndFunction(true); // The path is a directory
        }
        else
        {
            return pH->EndFunction(false); // The path is not a directory
        }
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        return pH->EndFunction(false, e.what()); // Return the error message if any exception occurs
    }
}

// -------------------------------------
int SC_LuaFileSystem::DirGetSize(IFunctionHandler* pH, const char* path)
{
    try {
        if (!std::filesystem::exists(path)) {
            return pH->EndFunction(0.f, "Directory does not exist.");
        }

        float totalSize = 0;

        for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
            if (entry.is_regular_file()) {
                totalSize += entry.file_size();
            }
        }

        return pH->EndFunction(totalSize);
    }
    catch (const std::filesystem::filesystem_error& e) {
        return pH->EndFunction(0.f, e.what());
    }
    catch (const std::exception& e) {
        return pH->EndFunction(0.f, e.what());
    }

    return pH->EndFunction(0.f, "Error");
}

// -------------------------------------
int SC_LuaFileSystem::DirExists(IFunctionHandler* pH, const char* path)
{
    try {
        std::filesystem::path dirPath(path);
        if (std::filesystem::exists(dirPath) && std::filesystem::is_directory(dirPath)) {
            return pH->EndFunction(true);
        }
        else {
            return pH->EndFunction(false, "Directory not found.");
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        return pH->EndFunction(false, e.what());
    }
    catch (const std::exception& e) {
        return pH->EndFunction(false, e.what());
    }

    pH->EndFunction(false, "Error");
}


// -------------------------------------
int SC_LuaFileSystem::DirCreate(IFunctionHandler* pH, const char* path)
{
    try {
        std::filesystem::path dirPath(path);
        if (std::filesystem::exists(dirPath)) {
            if (std::filesystem::is_directory(dirPath)) {
                return pH->EndFunction(true, "Directory already exists.");
            }
            else {
                return pH->EndFunction(false, "Path exists but is not a directory.");
            }
        }
        else {
            if (std::filesystem::create_directories(dirPath)) {
                return pH->EndFunction(true); // Directory created successfully
            }
            else {
                return pH->EndFunction(false, "Failed to create the directory.");
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        return pH->EndFunction(false, e.what());
    }
    catch (const std::exception& e) {
        return pH->EndFunction(false, e.what());
    }

    pH->EndFunction(false, "Error");
}

// -------------------------------------
int SC_LuaFileSystem::DirGetFiles(IFunctionHandler* pH, const char* path, int type)
{
    // ----------------------
    const char* filter = ".*";
    if (pH->GetParamCount() > 2)
        pH->GetParam(3, filter);

    EFileSystemGetFiles file_type = static_cast<EFileSystemGetFiles>(type);

    // ----------------------
    SmartScriptTable pResults(m_pSS);

    // ----------------------
    std::vector<std::string> collectedItems;
    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {


            std::string itemName = entry.path().filename().string();

            if (!Regexp(itemName, filter)) {
                continue;
            }

            switch (file_type) {
            case GETFILES_ALL:
                collectedItems.push_back(entry.path().string());
                break;
            case GETFILES_DIR:
                if (entry.is_directory()) {
                    collectedItems.push_back(entry.path().string());
                }
                break;
            case GETFILES_FILES:
                if (entry.is_regular_file()) {
                    collectedItems.push_back(entry.path().string());
                }
                break;

            default:
                gSC_Server->Log("bad type %d", type);
                break;
            }
        }
    }
    catch (const std::filesystem::filesystem_error& e) {
        gSC_Server->LogError("FileSystem Error: %s", e.what());
        return pH->EndFunction(pResults, e.what());
    }
    catch (const std::exception& e) {
        gSC_Server->LogError("FileSystem Exception: %s", e.what());
        return pH->EndFunction(pResults, e.what());
    }

    // ----------------------
    for (const auto& item : collectedItems) {
        pResults->PushBack(item.c_str());
    }

    // ----------------------
    return pH->EndFunction(pResults);
}