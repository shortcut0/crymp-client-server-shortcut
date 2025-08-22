#pragma once

#include "CryCommon/CryScriptSystem/IScriptSystem.h"

#include <regex>

enum EFileSystemGetFiles {
	GETFILES_ALL,
	GETFILES_DIR,
	GETFILES_FILES
};

class SC_LuaFileSystem : public CScriptableBase
{

	IScriptSystem *m_pSS;

public:

	// ------------
	SC_LuaFileSystem();
	~SC_LuaFileSystem();

	// ------------
	int FileIsFile(IFunctionHandler* pH, const char* path);
	int FileGetName(IFunctionHandler* pH, const char* path);
	int FileGetPath(IFunctionHandler* pH, const char* path);
	int FileGetSize(IFunctionHandler* pH, const char* path);
	int FileExists(IFunctionHandler* pH, const char* path);
	int FileDelete(IFunctionHandler* pH, const char* path);

	// ------------
	int DirGetName(IFunctionHandler* pH, const char* path);
	int DirIsDir(IFunctionHandler* pH, const char* path);
	int DirGetSize(IFunctionHandler* pH, const char* path);
	int DirExists(IFunctionHandler* pH, const char* path);
	int DirCreate(IFunctionHandler* pH, const char* path);
	int DirGetFiles(IFunctionHandler* pH, const char* path, int type);

private:

	// ------------
	bool Regexp(const std::string& itemName, const std::string& filter);
};
