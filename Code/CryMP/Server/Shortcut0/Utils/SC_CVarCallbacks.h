#pragma once

#ifndef  __SCCVAR_CALLBACKS__
#define  __SCCVAR_CALLBACKS__

#include "..\SC_Server.h"

class SC_ServerCCallbacks
{
public:
	static void Sv_MaxPlayers(ICVar* pCVar) {
		int num = pCVar->GetIVal();
		if (num > 99)
			gSC_Server->LogWarning("Max Players >99 won't be displayed correctly.");

		gSC_Server->UpdateGameSpyServerReport(SC_Server::EGameSpyUpdateType::eGSUpdate_Server, "maxplayers", pCVar->GetString());
	}

	static void Sv_ServerName(ICVar* pCVar) {

		const char* ServerName = pCVar->GetString();
		gSC_Server->UpdateGameSpyServerReport(SC_Server::EGameSpyUpdateType::eGSUpdate_Server, "hostname", ServerName);
		if (CGameRules* pGR = g_pGame->GetGameRules())
		{
		}
	}

	static void NewMapCommand(IConsoleCmdArgs *pArgs)
	{
		if (pArgs->GetArgCount() == 0)
		{
			gSC_Server->Log("Please specify Map Name");
			return;
		}

		const char* Map = pArgs->GetArg(1);
		if (Map == 0 || Map[0] == '\0')
		{
			gSC_Server->Log("Invalid Map Name");
			return;
		}

		gSC_Server->Log("Loading Level %s", Map);
		g_pGame->GetIGameFramework()->GetILevelSystem()->LoadLevel(Map);
	}

	static void OnCVarChanged(ICVar* pCVar)
	{
		int Value = pCVar->GetIVal();
		gSC_Server->GetEvents()->Call(SERVER_SCRIPT_EVENT_OnCVarChanged, pCVar->GetName(), Value);
	}
};

#endif // ! __CVAR_CALLBACKS__
