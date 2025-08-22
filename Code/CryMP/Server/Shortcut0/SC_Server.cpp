// ===================================================================================
//          ____            __  __ ____            ____                             --
//         / ___|_ __ _   _|  \/  |  _ \          / ___|  ___ _ ____   _____ _ __   --
//        | |   | '__| | | | |\/| | |_) |  _____  \___ \ / _ \ '__\ \ / / _ \ '__|  --
//        | |___| |  | |_| | |  | |  __/  |_____|  ___) |  __/ |   \ V /  __/ |     --
//         \____|_|   \__, |_|  |_|_|             |____/ \___|_|    \_/ \___|_|     --
//                    |___/          by: shortcut0                                  --
// ===================================================================================

#include "Library/StdFile.h"

#include "CryMP/Common/Executor.h"
#include "CryMP/Common/HTTPClient.h"
#include "CryCommon/CrySystem/IConsole.h"
#include "CryCommon/CrySystem/ISystem.h"
#include "CryGame/Game.h"
#include "CryGame/GameRules.h"
#include "CryGame/GameCVars.h"

#include "CryGame/Items/Weapons/Projectiles/Rocket.h"

#include "CryCommon/CryAction/IVehicleSystem.h"
#include "CryCommon/CryAction/IItemSystem.h"
#include "CryGame/Items/Weapons/WeaponSystem.h"


// Exit handler
#include "Utils\SC_QuitHook.h"
#include <cstdlib>  // For std::atexit

#include "SC_Server.h"
#include "SC_ServerLog.h"
#include "SC_ServerCVars.h"
#include "SC_ServerEvents.h"

#include "Library/StringTools.h"
#include "Library/Util.h"
#include "Library/WinAPI.h"
#include "Launcher/Resources.h"
#include "config.h"
#include "Version.h"

// File System
#include <iostream>
#include <fstream>
#include <string>


// deug..
#include "CryCommon/CryAction/IGameRulesSystem.h"

// ----------
SC_ServerCVars* g_pSC_ServerCVars = 0;

// -------------------------------------
SC_Server::SC_Server(IGameFramework* pFW, ISystem* pSystem) :
	m_Initialized(false)
{
	Init(pFW);
	gSC_Server = this;
}

// -------------------------------------
SC_Server::~SC_Server()
{
}

// -------------------------------------
void SC_Server::Quit()
{
	m_pExitHandler();
}

// -------------------------------------
void SC_Server::m_pExitHandler() {

	if (!gSC_Server)
		return;

	gSC_Server->QuitScript();
}

// -------------------------------------
void SC_Server::Init(IGameFramework* pGameFramework)
{

	// --------------------------------------
	Log("Setting atexit callback..");
	SC_QuitHook::Init();
	//std::atexit(m_pExitHandler);

	// --------------------------------------
	IConsole* pConsole = gEnv->pConsole;
	ISystem* pSystem = gEnv->pSystem;
	IScriptSystem* pScriptSystem = gEnv->pScriptSystem;
	IEntitySystem* pEntitySystem = gEnv->pEntitySystem;

	m_rootDir = std::filesystem::canonical(gEnv->pSystem->GetRootFolder());
	m_workingDir = std::filesystem::current_path();

	if (m_rootDir.empty())
		m_rootDir = m_workingDir;

	// -----------
	// Log
	Log("# ===================================================================================");
	Log("#          ____            __  __ ____            ____                             --");
	Log("#         / ___|_ __ _   _|  \\/  |  _ \\          / ___|  ___ _ ____   _____ _ __   --");
	Log("#        | |   | '__| | | | |\\/| | |_) |  _____  \\___ \\ / _ \\ '__\\ \\ / / _ \\ '__|  --");
	Log("#        | |___| |  | |_| | |  | |  __/  |_____|  ___) |  __/ |   \\ V /  __/ |     --");
	Log("#         \\____|_|   \\__, |_|  |_|_|             |____/ \\___|_|    \\_/ \\___|_|     --");
	Log("#                    |___/          by: shortcut0                                  --");
	Log("# ===================================================================================");

	Log(" ");
	Log(" ");
	Log(">    Root Dir: %s", m_rootDir.string().c_str());
	Log("> Working Dir: %s", m_workingDir.string().c_str());
	Log("> Initializing Now ...");

	// -----------
	this->m_pGSMaster = GetISystem()->GetINetwork()->GetService("GameSpy");
	this->m_pGameFramework = pGameFramework;
	this->m_pSS = gEnv->pScriptSystem;


	// -----------
	this->m_pExecutor = std::make_unique<Executor>();
	this->m_pHttpClient = std::make_unique<HTTPClient>(*this->m_pExecutor);

	this->m_pEventsCallback = std::make_unique<SC_ServerEvents>();
	this->m_pLuaFileSystem = std::make_unique<SC_LuaFileSystem>();

	this->m_pAC = std::make_unique<SC_ServerAnticheat>();
	this->m_pServerUtils = std::make_unique<SC_ServerUtils>();
	this->m_pCVars = std::make_unique<SC_ServerCVars>();
	this->m_pServerStats = std::make_unique<SC_ServerStats>();
	this->m_pServerTimer = std::make_unique<SC_ServerTimer>();

	this->m_pScriptBindServer = std::make_unique<ScriptBind_SC_Server>();
	this->m_pServerVehicleScriptBind = std::make_unique<CScriptBind_SC_ServerVehicle>(pSystem, pGameFramework);
	this->m_pServerGunTurretScriptBind = std::make_unique<CScriptBind_SC_GunTurret>(pSystem, pGameFramework);


	m_pScriptBindServer->SetPServer(this);
	//m_pEventsCallback->SetPServer(this);
	// -----------
	pGameFramework->RegisterListener(this, "crymp-sc-server", FRAMEWORKLISTENERPRIORITY_DEFAULT);
	pGameFramework->GetILevelSystem()->AddListener(this);
	pEntitySystem->AddSink(this);

	// --------------
	// Init AntiCheat
	m_pAC->Init(pGameFramework, GetEvents());


	// -----------
	// Init Game
	//m_pGame->Init(pGameFramework);


	// ========== CFG =============
	std::string cfg(m_ServerFolder + "\\Config\\Init.cfg");
	if (WinAPI::CmdLine::HasArg("-headless"))
	{
		cfg = m_ServerFolder + "\\Config\\Init-Headless.cfg";
	}

	ReadCfg(cfg);
	SetCVars();
	//set cvars

	// -----------
	// Init utils and cvars

	m_AStarTimer = SC_ServerTimer(1.f);
	m_ExplosionTimer = SC_ServerTimer(1.f);
	m_StatsTimer = SC_ServerTimer(1.f);

	m_DebugTimer = SC_ServerTimer(0.1f);
	m_DebugTimer2 = SC_ServerTimer(0.0f);
	m_DebugTimer3 = SC_ServerTimer(0.0f);

	m_ScriptFrameTimer = SC_ServerTimer(1 / 21); // 21 times per second
	m_ScriptSecondTimer = SC_ServerTimer(1.f);
	m_ScriptMinuteTimer = SC_ServerTimer(60.f);
	m_ScriptHourTimer = SC_ServerTimer(3600.f);

	m_pServerUtils->Init();

	InitCommands(pConsole);
	InitCVars(pConsole);

	InitMasters();



	m_Initialized = true;
	m_tickGoal = 1.f;
	m_minGoal = 60.f;
	m_hourGoal = 3600.f;
	m_lastChannel = 1;


	m_PreviousRate = 0.f;
	m_IdleRate = 5.f; // FIXME
	m_IsIdle = false;
	m_MaxRateCVar = pConsole->GetCVar("sv_dedicatedMaxRate");

	// --------------------------------
	// OVERWRITE GAME SCRIPT FILES
	const std::string overwriteFile = m_rootDir.string() + "/Config/GameOverwrites.cfg";
	InitGameOverwrites(overwriteFile);
	//LoadScript();

	// Enable network multithreadding
	// gEnv->pNetwork->EnableMultithreading(true);

	// -----------
	// Load Script File
	LoadScriptConfig();
	RegisterScriptGlobals();
	LoadScript();
}

// -------------------------------------
void SC_Server::DebugExplosion(const ExplosionInfo& info)
{
	//float diff = m_DebugTimer2.Diff_Refresh();
	//Log("explosion diff: %f effect: %s", diff, info.effect_name.c_str());
}

// -------------------------------------
bool SC_Server::ProcessExplosion(const ExplosionInfo& info)
{
	if (m_ExplosionTimer.Expired()) // 1 second has passed
	{
		m_ProcessedExplosions = 0;  // Reset the explosion counter for the new second
		m_ExplosionTimer.Refresh(); // Refresh the timer for the next cycle
	}

	m_ProcessedExplosions++;  // Increment the explosion count for the current second

	//Log("counter: %d", m_ProcessedExplosions);

	// Return whether the number of processed explosions is within the allowed limit
	return (m_ProcessedExplosions <= GetCVars()->server_explosion_Limiter);
}

// -------------------------------------
void SC_Server::SpawnDebugParticle(Vec3 pos, Vec3 dir, float scale)
{
	GetEvents()->Call("g_gameRules.game.ServerExplosion", ScriptHandle(EntityId(0)), ScriptHandle(EntityId(0)), 0, pos, dir, 0, 0, 0, 0, "explosions.flare.a", scale, 0, 0, 0, 0);
}

// -------------------------------------
void SC_Server::Debug()
{
	IEntityItPtr pIIt = gEnv->pEntitySystem->GetEntityIterator();
	IEntity* pEntity = 0;

	pIIt->MoveFirst();

	IGameRulesSystem* pGR = gEnv->pGame->GetIGameFramework()->GetIGameRulesSystem();
	IGameRules* pGRR = pGR->GetCurrentGameRules();
	//CGameRules* pCGR = static_cast<CGameRules*>(pGRR);

	g_pGame->GetIGameFramework()->GetIGameRulesSystem();

	while (pEntity = pIIt->Next())
	{
		if (strcmp("Player", pEntity->GetClass()->GetName()) == 0)
		{
			GetEvents()->Call("ServerCommands.DebugEffect", pEntity->GetScriptTable(), "/flare");
			GetEvents()->Call("g_gameRules.game.ServerExplosion", ScriptHandle(EntityId(0)), ScriptHandle(EntityId(0)), 0, pEntity->GetWorldPos(), Vec3(0, 0, 1), 0, 0, 0, 0, "explosions.flare.a", 1, 0, 0, 0, 0);

			/*
			IScriptTable* gr;
			IScriptTable* grg;
			gEnv->pScriptSystem->GetGlobalValue("g_gameRules", gr);
			if (!gr)
			{
				Log("not found yet");
				return;
			}
			gr->GetValue("game", grg);
			HSCRIPTFUNCTION spawn;
			grg->GetValue("ServerExplosion", spawn);

			//Script::Call(gEnv->pScriptSystem, spawn, 0, 0, 0, pEntity->GetWorldPos(), Vec3(0, 0, 1), 0, 0, 0, 0, "explosions.flare.a", 1, 0, 0, 0, 0);

			gEnv->pScriptSystem->BeginCall(spawn);
			gEnv->pScriptSystem->PushFuncParam(grg);
			gEnv->pScriptSystem->PushFuncParam(0);
			gEnv->pScriptSystem->PushFuncParam(0);
			gEnv->pScriptSystem->PushFuncParam(0);
			gEnv->pScriptSystem->PushFuncParam(pEntity->GetWorldPos());
			gEnv->pScriptSystem->PushFuncParam(Vec3(0,0,1));
			gEnv->pScriptSystem->PushFuncParam(0);
			gEnv->pScriptSystem->PushFuncParam(0);
			gEnv->pScriptSystem->PushFuncParam(0);
			gEnv->pScriptSystem->PushFuncParam(0);
			gEnv->pScriptSystem->PushFuncParam("explosions.flare.a");
			gEnv->pScriptSystem->PushFuncParam(1);
			gEnv->pScriptSystem->PushFuncParam(0);
			gEnv->pScriptSystem->PushFuncParam(0);
			gEnv->pScriptSystem->PushFuncParam(0);
			gEnv->pScriptSystem->PushFuncParam(0);

			// 0, 0, 0, 0, "explosions.flare.a", 1, 0, 0, 0, 0
			gEnv->pScriptSystem->EndCall();

			gEnv->pScriptSystem->ReleaseFunc(spawn);

			// g_pGame:ServerExplosion(NULL_ENTITY, NULL_ENTITY, 0, (vPos), (vDir or vectors.up), 0, 0, 0, 0, sEffect, (iScale or 1), nil, 0, 0, 0)
			*/
			HitInfo sh;
			sh.shooterId = pEntity->GetId();
			sh.targetId = pEntity->GetId();
			sh.remote = true;
			sh.type = 0;
			sh.weaponId = pEntity->GetId();
			sh.pos = pEntity->GetWorldPos();
			sh.dir = Vec3(0, 0, 0);
			sh.fmId = 1;
			sh.bulletType = 1;
			sh.angle = 45;
			sh.damage = 999;
			sh.material = 1;
			sh.normal = Vec3(0, 0, 0);
			sh.seq = 1;
			sh.radius = 1;
			sh.projectileId = 0;
			pGRR->ClientHit(sh);
			Log("hit %s", pEntity->GetName());
		}
	}
}

// -------------------------------------
void SC_Server::Update(float dT)
{


	// ========== UPDATE =============
	float frametime = gEnv->pTimer->GetFrameTime();

	if (m_Initialized) {

		bool ScriptReady = IsLuaReady();


		if (GetCVars()->server_debug_positions > 0 && m_DebugTimer.Expired_Refresh())
		{
			Debug();
		}

		if (GetCVars()->filesave_debug && m_DebugTimer3.Expired_Refresh())
		{
			GetFileSaver()->SaveFile("testSaveFile.txt", "hello-" + std::to_string(gEnv->pTimer->GetFrameTime()));
		}

		if (m_StatsTimer.Expired_Refresh())
		{
			GetStats()->Update(); // update every second to show proper CPU usage, longer times will give more accurate results
		}

		if (ScriptReady)
		{

			// Change Server Rate to Idle rate if no channels exist to preserve CPU and Resources
			if (m_MaxRateCVar)
			{
				float iMaxRate = m_MaxRateCVar->GetFVal();
				if (m_GameChannels.empty())
				{
					if (!m_IsIdle)
					{
						m_IsIdle = true;
						m_MaxRateCVar->Set(m_IdleRate);
						m_PreviousRate = iMaxRate;
						Log("Entering Idle State..");
					}
				}
				else if (m_IsIdle)
				{
					m_IsIdle = false;
					m_MaxRateCVar->Set(m_PreviousRate);
					Log("Exiting Idle State..");
				}
			}

			while (INetChannel* pNextChannel = g_pGame->GetIGameFramework()->GetNetChannel(m_lastChannel))
			{
				const std::string ip = GetUtils()->CryChannelToIP(m_lastChannel);
				GetEvents()->Call(SERVER_SCRIPT_EVENT_OnConnection, m_lastChannel, ip.c_str());
				m_GameChannels.push_back(m_lastChannel);
				m_lastChannel++;
			}

			// Every Frame
			if (m_ScriptFrameTimer.Expired_Refresh())
			{
				GetEvents()->Call(SERVER_SCRIPT_EVENT_OnUpdate);
			}

			// Every Second
			if (m_ScriptSecondTimer.Expired_Refresh())
			{
				GetEvents()->Call(SERVER_SCRIPT_EVENT_OnTimer, 1);

			}

			// Every Minute
			if (m_ScriptMinuteTimer.Expired_Refresh())
			{
				GetEvents()->Call(SERVER_SCRIPT_EVENT_OnTimer, 2);
			}

			// Every Hour
			if (m_ScriptHourTimer.Expired_Refresh())
			{
				GetEvents()->Call(SERVER_SCRIPT_EVENT_OnTimer, 3);
			}

			// Check if System is Quitting
			if (gEnv->pSystem->IsQuitting())
			{
				QuitScript();
			}
		}
	}

	if (m_pSS)
	{
		m_pSS->SetGlobalValue("DLL_ERROR", false);
	}

}

// -------------------------------------
void SC_Server::UpdateEntityRemoval()
{

	float frameTime = gEnv->pTimer->GetCurrTime();
	IEntitySystem* pEntitySystem = gEnv->pEntitySystem;

	if (m_scheduledEntityRemovals.empty())
		return;

	// Iterate and remove EntityId 6789
	for (auto it = m_scheduledEntityRemovals.begin(); it != m_scheduledEntityRemovals.end(); /* no increment here */) {
		if (it->second >= frameTime) {  // Access the EntityId using it->first

			//FIXME
			//pEntitySystem->RemoveEntity(it->first);
			it = m_scheduledEntityRemovals.erase(it);  // Remove element and get new iterator
		}
		else {
			++it;  // Move to the next element
		}
	}
}

// -------------------------------------
void SC_Server::InitCommands(IConsole* pConsole)
{
	pConsole->AddCommand("server_reloadScript", OnInitLuaCmd, VF_NOT_NET_SYNCED, "Reload the main Server Initializer Script");
}

// -------------------------------------
void SC_Server::InitCVars(IConsole* pConsole)
{
	g_pSC_ServerCVars = GetCVars();
	g_pSC_ServerCVars->InitCVars(pConsole);

	//pConsole->Register("fff", &test_cvar_, 0, 0, "Enables debugging and testing the ASYNC server file saver");
	//g_pServerCVars->RegisterCVar("fff", &test_cvar_, 50);
}

// -------------------------------------
void SC_Server::SetCVars()
{

	IConsole* pConsole = gEnv->pConsole;
	if (!pConsole)
		return;


	// We do this. The Name will later be changed by the scripts configuration
	if (ICVar* pServerName = pConsole->GetCVar("sv_serverName"))
	{
		pServerName->Set("Server Starting Up...");
	}
}

// -------------------------------------
bool SC_Server::ReadCfg(const std::string& filepath)
{
	std::ifstream file(filepath);
	if (!file.is_open()) {
		LogError("Could not open the config file \"%s\"", filepath.c_str());
		return false;
	}

	IConsole* pConsole = gEnv->pConsole;
	SC_ServerUtils* pUtils = GetUtils();
	bool LevelInitialized = false;
	//bool ServerNameFound = false;

	std::string line;
	while (std::getline(file, line)) {

		// we parse the line for comments!
		std::string CleanedLine = pUtils->StripCfgComment(line);
		if (!CleanedLine.empty())
		{
			//LogDebug("orig:%s", line.c_str());
			//LogDebug("exec:%s", CleanedLine.c_str());
			pConsole->ExecuteString(CleanedLine.c_str());

			LevelInitialized = (LevelInitialized || std::regex_match(CleanedLine, std::regex("^\\s*g_nextlevel\\s+x\\s*$", std::regex::icase)));
			//ServerNameFound = (ServerNameFound || std::regex_match(CleanedLine, std::regex("^\\s*sv_servername\\s*=\\s*.*$", std::regex::icase)));
		}
		else
		{
			//LogDebug("Skipping comment: '%s'", line.c_str());
		}
	}


	file.close();

	if (file.fail() && !file.eof()) {
		LogError("An error occurred while reading the config file \"%s\"", filepath.c_str());
		return false;
	}

	// no g_nextlevel found. without this, the server will be stuck in a half-initialized limbo state!
	if (!LevelInitialized)
	{
		Log("[LazyBum] Initializing Level");

		ICVar* pMapVar = pConsole->GetCVar("sv_Map");
		ICVar* pRulesVar = pConsole->GetCVar("sv_gameRules");
		std::string Map = (pMapVar->GetString());

		//Log("%s", g_pGame->GetIGameFramework()->GetILevelSystem()->GetCurrentLevel()->GetLevelInfo()->GetName());
		//Log("mMap=%s", Map.c_str());

		if (Map.empty() || Map == "ps_port")
		{
			//pMapVar->Set("multiplayer/ps/mesa");
			//pRulesVar->Set("PowerStruggle");
			//gEnv->pConsole->ExecuteString("map multiplayer/ps/mesa x");
			//g_pGame->GetIGameFramework()->ExecuteCommandNextFrame("map multiplayer/ps/mesa x");
		}

		//gEnv->pConsole->ExecuteString("g_nextLevel x");
	}

	return true;
}

// -------------------------------------
void SC_Server::InitMasters()
{
	std::string content;

	if (StdFile file("masters.txt", "r"); file.IsOpen())  // Crysis main directory
	{
		CryLogAlways("$6[CryMP] Using local masters.txt as the master server list provider");

		content = file.ReadAll();
	}
	else
	{
		content = WinAPI::GetDataResource(nullptr, RESOURCE_MASTERS_TXT);
	}

	for (const std::string_view& master : Util::SplitWhitespace(content))
	{
		m_masters.emplace_back(master);
	}

	if (m_masters.empty())
	{
		m_masters.emplace_back("crymp.org");
	}

	// OLD CODE
	/*
	std::string content;
	WinAPI::File file("masters.txt", WinAPI::FileAccess::READ_ONLY);  // Crysis main directory
	if (file)
	{
		CryLogAlways("$6[CryMP] Using local masters.txt as the master server list provider");

		try
		{
			content = file.Read();
		}
		catch (const std::exception& ex)
		{
			CryLogAlways("$4[CryMP] Failed to read the masters.txt file: %s", ex.what());
		}
	}
	else
		content = WinAPI::GetDataResource(nullptr, RESOURCE_MASTERS);

	for (const std::string_view& master : Util::SplitWhitespace(content))
		m_masters.emplace_back(master);

	if (m_masters.empty())
		m_masters.emplace_back("crymp.org");
		*/
}

// -------------------------------------
std::string SC_Server::GetMasterServerAPI(const std::string& master)
{

	const std::string HTTP = (m_UseOnlyHTTP ? "http://" : "https://");
	if (master.empty())
	{
		return HTTP + m_masters[0] + "/api";
	}
	else
	{
		int a = 0, b = 0, c = 0, d = 0;
		// in case it is IP, never use HTTPS
		if (sscanf(master.c_str(), "%d.%d.%d.%d", &a, &b, &c, &d) == 4 || master == "localhost")
			return "http://" + master + "/api";
		else
			return HTTP + master + "/api";
	}
}

// -------------------------------------
void SC_Server::HttpGet(const std::string_view& url, std::function<void(HTTPClientResult&)> callback)
{
	HTTPClientRequest request;
	request.method = "GET";
	request.url = url;
	request.callback = std::move(callback);

	HttpRequest(std::move(request));
}

// -------------------------------------
void SC_Server::HttpRequest(HTTPClientRequest&& request)
{
	for (const std::string& master : m_masters)
	{
		if (Util::StartsWith(request.url, GetMasterServerAPI(master)))
		{
			break;
		}
	}

	//Log("HTTP(%s): %s", request.url.c_str(), request.data.c_str());
	m_pHttpClient->Request(std::move(request));
}

// -------------------------------------
void SC_Server::OnInitLuaCmd(IConsoleCmdArgs* pArgs) {

	//gServer->LoadScript(); // normal
	//gServer->InitScript(false);
	//gServer->InitScript(true);
	// 
	gSC_Server->LoadScript();		// Reload
	gSC_Server->InitScript(false);  // Init()
	gSC_Server->InitScript(true);	// PostInit()

	//ISSM* pSSM = g_pGame->GetSSM();
	
	// SC FIxme
	//pSSM->GetSCServer()->LoadScript();
	//pSSM->GetSCServer()->InitScript(false);
	//pSSM->GetSCServer()->InitScript(true);
}

// -------------------------------------
void SC_Server::RegisterScriptGlobals() {

	m_pSS->SetGlobalValue("CRYMP_SERVER_EXE", CRYMP_SERVER_EXE_NAME);
	m_pSS->SetGlobalValue("CRYMP_SERVER_BITS", CRYMP_BITS);
	m_pSS->SetGlobalValue("CRYMP_SERVER_VERSION", CRYMP_VERSION);
	m_pSS->SetGlobalValue("CRYMP_SERVER_VERSION_STRING", CRYMP_VERSION_STRING);
	m_pSS->SetGlobalValue("CRYMP_SERVER_BUILD_TYPE", CRYMP_BUILD_TYPE);
	m_pSS->SetGlobalValue("CRYMP_SERVER_COMPILER", CRYMP_COMPILER);
	
	m_pSS->SetGlobalValue("ePMCInfo_PageFaultCount", SC_ServerStats::PMCInfo::PageFaultCount);
	m_pSS->SetGlobalValue("ePMCInfo_PagefileUsage", SC_ServerStats::PMCInfo::PagefileUsage);
	m_pSS->SetGlobalValue("ePMCInfo_PeakPagefileUsage", SC_ServerStats::PMCInfo::PeakPagefileUsage);
	m_pSS->SetGlobalValue("ePMCInfo_QuotaNonPagedPoolUsage", SC_ServerStats::QuotaNonPagedPoolUsage);
	m_pSS->SetGlobalValue("ePMCInfo_QuotaPagedPoolUsage", SC_ServerStats::PMCInfo::QuotaPagedPoolUsage);
	m_pSS->SetGlobalValue("ePMCInfo_QuotaPeakNonPagedPoolUsage", SC_ServerStats::PMCInfo::QuotaPeakNonPagedPoolUsage);
	m_pSS->SetGlobalValue("ePMCInfo_WorkingSetSize", SC_ServerStats::PMCInfo::WorkingSetSize);
	m_pSS->SetGlobalValue("ePMCInfo_PeakWorkingSetSize", SC_ServerStats::PMCInfo::PeakWorkingSetSize);

	m_pSS->SetGlobalValue("ActorCheat_UnlimitedAmmo", CActor::SvCheatModes::UnlimitedAmmo);
	m_pSS->SetGlobalValue("ActorCheat_UnlimitedItems", CActor::SvCheatModes::UnlimitedItems);
	m_pSS->SetGlobalValue("ActorCheat_RapidFire", CActor::SvCheatModes::RapidFire);

	m_pSS->SetGlobalValue("LEAN_RIGHT", 1); 
	m_pSS->SetGlobalValue("LEAN_LEFT", -1);

	m_pSS->SetGlobalValue("GSUpdate_Server", (int)SC_Server::EGameSpyUpdateType::eGSUpdate_Server);
	m_pSS->SetGlobalValue("GSUpdate_Player", (int)SC_Server::EGameSpyUpdateType::eGSUpdate_Player);
	m_pSS->SetGlobalValue("GSUpdate_Team", (int)SC_Server::EGameSpyUpdateType::eGSUpdate_Team);
}

// -------------------------------------
void SC_Server::QuitScript() {

	if (m_ScriptQuitting || !m_pSS || !IsLuaReady())
	{
		return;
	}

	m_ScriptQuitting = GetEvents()->Call(SERVER_SCRIPT_EVENT_OnGameShutdown);
}

// -------------------------------------
void SC_Server::LoadScriptConfig() {

	SC_ServerEvents* pEvents = GetEvents();
	if (!pEvents)
		return; // impossible scenario

	// Config File (maybe a cvar for custom location?)
	const std::string ConfigFile = (m_rootDir.string() + "\\Config\\Server_CPPConfig.lua");
	if (!m_pSS->ExecuteFile(ConfigFile.c_str()))
	{
		Log("Failed to Load the C++ Config File.");
		return;
	}

	// read config
	const char* Config_ScriptPath;
	if (!m_pSS->GetGlobalValue("Server_CPPConfig.ServerScript", Config_ScriptPath))
	{
		m_ScriptPath = ("/Scripts/Server.lua");
		Log("Using default Script path");
	}
	else
	{
		m_ScriptPath = Config_ScriptPath;
		Log("Using custom Script Path");
	}

	m_ScriptPath = (m_rootDir.string() + m_ScriptPath);
	const char* Config_ScriptHost;
	if (!m_pSS->GetGlobalValue("Server_CPPConfig.EventHost", Config_ScriptHost))
	{
		pEvents->ScriptHost = "Server.Events.Callbacks";
		Log("Using default Event Host");
	}
	else
	{
		pEvents->ScriptHost = Config_ScriptHost;
		Log("Using custom Script Host");
	}

	bool UseOnlyHTTP = false;
	if (!m_pSS->GetGlobalValue("Server_CPPConfig.ForceHTTPOverHTTPs", UseOnlyHTTP))
	{
		Log("Using default HTTP/s Setting");
	}
	else
	{
		m_UseOnlyHTTP = UseOnlyHTTP;
		Log("Forcing HTTP Over HTTPs: %s", (UseOnlyHTTP ? "true" : "false"));
	}

	bool UseAutomaticScriptRedirection = false;
	if (!m_pSS->GetGlobalValue("Server_CPPConfig.UseAutomaticScriptRedirection", UseAutomaticScriptRedirection))
	{
		Log("Not using automatic Script Redirecting");
	}
	else
	{
		m_UseAutomaticScriptRedirection = UseAutomaticScriptRedirection;
		Log("Automatic Script-Redirecting: %s", (UseOnlyHTTP ? "true" : "false"));
	}

	Log("CPP Config Loaded");


}

// -------------------------------------
void SC_Server::LoadScript(bool ForceInit) {


	const char* ScriptFile = m_ScriptPath.c_str();

	Log("Loading Server Script: %s", ScriptFile);
	bool WasLoaded = m_ScriptLoaded;
	if (WasLoaded && m_ScriptInitialized)
	{
		bool PreInitialize(false);
		if (GetEvents()->Get("Server.PreInitialize", PreInitialize) && !PreInitialize)
		{
			LogError("Failed to Pre-Initialize the Script!");
			m_ScriptLoaded = false;
			return;
		}
	}

	m_ScriptLoaded = m_pSS->ExecuteFile(ScriptFile, true, true);

	if (!m_ScriptLoaded)
	{
		return LogError("Failed to load the Server Script: %s", ScriptFile);
	}

	if (!WasLoaded)
	{
		InitScript(false); // NOT Post!
	}

	if (ForceInit)
	{
		InitScript();

	}

	Log("Server Script Loaded");
}

// -------------------------------------
void SC_Server::InitScript(bool PostInitialize) {

	if (PostInitialize and !m_ScriptInitialized)
	{
		return LogError("Cannot Post-Initialize because Script is not Initialized yet!");
	}


	const char* PostString = (PostInitialize ? "Post-" : "");
	Log("%sInitializing Server Script..", PostString);

	if (!m_ScriptLoaded)
	{
		return LogWarning("Script not loaded: Cannot %sInitialize!", PostString);
	}

	bool Success(false);



	m_ScriptInitialized = Success; // for event handler
	m_ScriptInitialized = (GetEvents()->Get((PostInitialize ? "Server.PostInitialize" : "Server.Initialize"), Success) && Success);

	if (!m_ScriptInitialized)
	{
		return LogError("Failed to %sInitialize the Script!", PostString);
	}



	if (PostInitialize)
	{
		ScriptInitEntities();
		m_ScriptPostInitialized = true;
		m_pSS->SetGlobalValue("SCRIPT_WAS_POST_INITIALIZED", true);
	}
	else
	{
		m_pSS->SetGlobalValue("SCRIPT_WAS_INITIALIZED", true);
	}

	Log("Server Script %sInitialized", PostString);
	m_pSS->SetGlobalValue("SCRIPT_ERROR", false);
	return;
}

// -------------------------------------
void SC_Server::ScriptInitGlobals()
{
	/*if (!IsLuaReady())
	{
		return;
	}*/


	// PMC Info Keys

}

// -------------------------------------
void SC_Server::ScriptInitEntities()
{
	if (!IsLuaReady())
	{
		return;
	}

	if (m_PreSpawnedEntities.empty())
	{
		return;
	}

	for (const auto& entity : m_PreSpawnedEntities) {

		// Process each entry
		if (IEntity* pEntity = gEnv->pEntitySystem->GetEntity(entity.first))
		{
			PostInitEntityTypes pit = entity.second;
			bool vehicle = (pit == PostInitEntityTypes::ePIT_Vehicle);
			bool item = (pit == PostInitEntityTypes::ePIT_Item);
			bool actor = (pit == PostInitEntityTypes::ePIT_Actor);

			if (IScriptTable* pScriptTable = pEntity->GetScriptTable())
			{
				GetEvents()->Call(SERVER_SCRIPT_EVENT_OnEntitySpawn, pScriptTable, ScriptHandle(pEntity->GetId()), vehicle, item, actor, (int)pit);
			}
		}
	}

	Log("Synched %d Spawned Entities with the Script", m_PreSpawnedEntities.size());
	m_PreSpawnedEntities.clear();
}

// -------------------------------------
void SC_Server::OnPostUpdate(float deltaTime)
{
	FUNCTION_PROFILER(gEnv->pSystem, PROFILE_GAME);
	this->m_pExecutor->OnUpdate();
}

// -------------------------------------
void SC_Server::OnGameStart(IGameObject* pGameRules)
{
	// FIXME: Experimental!
	//gServer->LoadScript(true);
}

// -------------------------------------
void SC_Server::OnSaveGame(ISaveGame* saveGame)
{
}

// -------------------------------------
void SC_Server::OnLoadGame(ILoadGame* loadGame)
{
}

// -------------------------------------
void SC_Server::OnLevelEnd(const char* nextLevel)
{
}

// -------------------------------------
void SC_Server::OnActionEvent(const SActionEvent& event)
{
	FUNCTION_PROFILER(gEnv->pSystem, PROFILE_GAME);

	switch (event.m_event)
	{
	case eAE_channelCreated:
	{
		break;
	}
	case eAE_channelDestroyed:
	{
		auto it = std::find(m_GameChannels.begin(), m_GameChannels.end(), event.m_value);
		if (it != m_GameChannels.end())
		{
			m_GameChannels.erase(it);
		}
		break;
	}
	case eAE_connectFailed:
	case eAE_connected:
	case eAE_disconnected:
	case eAE_clientDisconnected:
	case eAE_resetBegin:
	case eAE_resetEnd:
	case eAE_resetProgress:
	case eAE_preSaveGame:
	case eAE_postSaveGame:
	case eAE_inGame:
	case eAE_serverName:
	case eAE_serverIp:
	case eAE_earlyPreUpdate:
	{
		break;
	}
	}
}

// -------------------------------------
void SC_Server::OnLevelNotFound(const char* levelName)
{
}

// -------------------------------------
void SC_Server::OnLoadingStart(ILevelInfo* pLevel)
{
	m_ScriptPostInitialized = false;
	m_pSS->SetGlobalValue("MAP_LOAD_START", gEnv->pTimer->GetCurrTime());
	gEnv->pScriptSystem->ForceGarbageCollection();
	if (IsLuaReady())
	{
		GetEvents()->Call(SERVER_SCRIPT_EVENT_OnLevelStart);
	}
}

// -------------------------------------
void SC_Server::OnLoadingComplete(ILevel* pLevel)
{

	//LoadScript(true); // i dont like to reload this file. so, instead, we will just initialize it here (post)
	InitScript();

	if (m_pSS)
	{
		m_pSS->SetGlobalValue("MAP_START_TIME", gEnv->pTimer->GetCurrTime());
		if (IsLuaReady()) {
			GetEvents()->Call(SERVER_SCRIPT_EVENT_OnMapStarted);
		}
	}


}

// -------------------------------------
void SC_Server::OnLoadingError(ILevelInfo* pLevel, const char* error)
{
}

// -------------------------------------
void SC_Server::OnLoadingProgress(ILevelInfo* pLevel, int progressAmount)
{
}


// -------------------------------------
bool SC_Server::OnBeforeSpawn(SEntitySpawnParams& params)
{
	FUNCTION_PROFILER(gEnv->pSystem, PROFILE_GAME);

	//CryMP: Archetype Loader
	if (params.sName && params.sName[0] == '*')
	{
		const char* name = params.sName + 1;
		const char* archetypeEnd = strchr(name, '|');

		if (archetypeEnd)
		{
			const std::string archetypeName(name, archetypeEnd - name);

			if (!gEnv->bServer)
			{
				params.sName = archetypeEnd + 1;
			}
			params.pArchetype = gEnv->pEntitySystem->LoadEntityArchetype(archetypeName.c_str());

			if (!params.pArchetype && params.pClass)
			{
				Log("Archetype '%s' not found for entity %s",
					archetypeName.c_str(),
					params.pClass->GetName()
				);

				if (gEnv->bServer) //If archetype loading failed, don't proceed spawn on the server
					return false;
			}
		}
	}
	//else if (IsLuaReady()) {
	//}

	// Fixed entity names for respawning entities
	const char* respawn_name;
	IScriptTable* pProperties = params.pPropertiesTable;
	if (pProperties)
	{
		Log("found props..");
		if (pProperties->GetValue("sRespawnName", respawn_name))
		{
			params.sName = respawn_name;
			Log("Fixed name for Entity %s", respawn_name);
		}
	}

	return true;
}


// -------------------------------------
void SC_Server::OnSpawn(IEntity* pEntity, SEntitySpawnParams& params)
{
	FUNCTION_PROFILER(gEnv->pSystem, PROFILE_GAME);

	// For -oldgame
	if (!g_pGame)
	{
		return;
	}

	//Log("name=%s", params.sName);
	//Log("class=%s", params.pClass->GetName());
	/*if (strcmp(params.sName, "GameRules") == 0 && (strcmp(params.pClass->GetName(), "PowerStruggle") + strcmp(params.pClass->GetName(), "InstantAction")) > 0)
	{
		gServer->LoadScript(true);
	}*/

	// =====================================
	bool item = false;
	bool ignore = false;
	bool vehicle = false;
	bool actor = false;
	if (IEntityClass* pClass = params.pClass)
	{

		// Vehicle
		if (m_pGameFramework->GetIVehicleSystem()->IsVehicleClass(pClass->GetName()))
		{
			vehicle = true;
			//GetEvents()->Call(SERVER_SCRIPT_EVENT_OnVehicleSpawn, ScriptHandle(pEntity->GetId()));
		}

		// Items
		if (m_pGameFramework->GetIItemSystem()->IsItemClass(pClass->GetName()))
		{
			item = true;
		}

		// Actors
		if (m_pGameFramework->GetIActorSystem()->GetActor(pEntity->GetId()))
		{
			actor = true;
		}

		// Filter out Projectiles
		if (g_pGame->GetWeaponSystem()->GetProjectile(pEntity->GetId()) != 0)
		{
			ignore = true;
		}
	}

	if (!ignore)
	{
		if (IScriptTable* pScriptTable = pEntity->GetScriptTable())
		{

			if (IsLuaReady() && m_ScriptPostInitialized)
			{
				GetEvents()->Call(SERVER_SCRIPT_EVENT_OnEntitySpawn, pScriptTable, ScriptHandle(pEntity->GetId()), vehicle, item, actor);
			}
			else
			{
				m_PreSpawnedEntities.push_back(std::make_pair(pEntity->GetId(), (actor ? PostInitEntityTypes::ePIT_Actor : vehicle ? PostInitEntityTypes::ePIT_Vehicle : item ? PostInitEntityTypes::ePIT_Item : PostInitEntityTypes::ePIT_None)));
				//m_PreSpawnedEntities.emlace();
			}
		}
	}

	// =====================================
	m_SpawnCounter++;
	m_LastSpawnId = pEntity->GetId();
}

// -------------------------------------
bool SC_Server::OnRemove(IEntity* pEntity)
{
	return true;
}

// -------------------------------------
void SC_Server::OnEvent(IEntity* pEntity, SEntityEvent& event)
{
}

// -------------------------------------
void SC_Server::OnScriptError(const std::string& error)
{
	if (true || IsLuaReady())
	{
		m_pSS->SetGlobalValue("SCRIPT_ERROR", true);
		GetEvents()->Call(SERVER_SCRIPT_EVENT_OnScriptError, error.c_str());
	}
}

// -------------------------------------
bool SC_Server::UpdateGameSpyServerReport(EGameSpyUpdateType type, const char* key, const char* value, int index) {

	INetworkService* pGS = GetGSMaster();
	if (!pGS || pGS == nullptr)
		return false;


	IServerReport* pGSReport = pGS->GetServerReport();
	if (!pGSReport)
		return false;

	switch (type)
	{
	case EGameSpyUpdateType::eGSUpdate_Server:
		pGSReport->SetServerValue(key, value);
		break;

	case EGameSpyUpdateType::eGSUpdate_Player:
		pGSReport->SetPlayerValue(index, key, value);
		break;

	case EGameSpyUpdateType::eGSUpdate_Team:
		pGSReport->SetTeamValue(index, key, value);
		break;

	default:
		Log("Unknown type specified to UpdateGameSpyServerReport(): %d", type);
		break;
	}

	pGSReport->Update();
	return true;
}

// -------------------------------------
void SC_Server::InitGameOverwrites(const std::string& filename) {

	SC_ServerUtils* pUtils = GetUtils();

	std::ifstream file(filename);
	std::string line;

	if (!file.is_open()) {
		LogError("Failed to open GameOverwrites File!");
		return;
	}

	const std::string root = m_rootDir.filename().string();
	while (std::getline(file, line)) {

		std::string CleanedLine = pUtils->StripCfgComment(line);
		if (CleanedLine.empty())
			continue;

		size_t pos = CleanedLine.find('=');
		if (pos != std::string::npos) {
			// Split the line into STRING1 and STRING2
			std::string game_original = CleanedLine.substr(0, pos);
			std::string game_replace = CleanedLine.substr(pos + 1);

			// Trim any whitespace around the strings (optional)
			game_original.erase(0, game_original.find_first_not_of(" \t"));
			game_original.erase(game_original.find_last_not_of(" \t") + 1);
			game_replace.erase(0, game_replace.find_first_not_of(" \t"));
			game_replace.erase(game_replace.find_last_not_of(" \t") + 1);

			// Add to the vector
			//Log("Added overwrite: %s = %s", game_original.c_str(), game_replace.c_str());
			//m_pOverwriteSF.push_back(std::make_pair(game_original, std::string(/*"../" +*/ root + "/" + game_replace)));
			m_pOverwriteSF.push_back(std::make_pair(game_original, std::string(m_rootDir.string() + "/" + game_replace))); // need to make it a full path!
		}
	}

	file.close();
}

// -------------------------------------
void SC_Server::OnLoadingScript(const std::string& fileName)
{

	if (!IsLuaReady())
	{
		return;
	}

	GetEvents()->Call(SERVER_SCRIPT_EVENT_OnLoadingScript, fileName.c_str());
}

// -------------------------------------
void SC_Server::OnScriptLoaded(const std::string& fileName, bool success)
{

	if (!IsLuaReady())
	{
		return;
	}

	GetEvents()->Call(SERVER_SCRIPT_EVENT_OnScriptLoaded, fileName.c_str(), success);
}


// -------------------------------------
bool SC_Server::OverwriteScriptPath(std::string &output, const std::string& input)
{


	std::string lowerInput = input;
	std::transform(lowerInput.begin(), lowerInput.end(), lowerInput.begin(), ::tolower);


	// Find the element with the matching key in the vector
	auto it = std::find_if(m_pOverwriteSF.begin(), m_pOverwriteSF.end(),
		[&lowerInput](const std::pair<std::string, std::string>& pair) {
			//CryLogAlways("compare %-30s->%s", pair.first.c_str(), lowerInput.c_str());
			std::string lowerFirst = pair.first;
			std::transform(lowerFirst.begin(), lowerFirst.end(), lowerFirst.begin(), ::tolower);
			return lowerFirst == lowerInput;
		});

	// If found, overwrite the path
	if (it != m_pOverwriteSF.end()) {
		output = it->second;  // Assign the new value as needed
		return true;
	}

	// Check if the file exists in our overwrite directory
	if (m_UseAutomaticScriptRedirection)
	{
		const std::string AutoOverwrite = m_rootDir.string() + "/Scripts/Game/Overwrites/" + lowerInput;
		if (GetUtils()->FileExists((AutoOverwrite)))
		{
			output = (AutoOverwrite);
			return true;
		}
	}
	return false;

}

// -------------------------------------
void SC_Server::OnClientEnteredGame(IGameRules* pGR, int channelId, bool isReset)
{
	GetEvents()->Call(SERVER_SCRIPT_EVENT_OnClientEnteredGame, channelId, isReset);
}

// -------------------------------------
void SC_Server::OnClientConnect(IGameRules* pGR, int channelId, bool isReset)
{
	CActor* pActor = g_pGame->GetGameRules()->GetActorByChannelId(channelId);
	if (pActor)
	{
		GetEvents()->Call(SERVER_SCRIPT_EVENT_OnClientConnect, channelId, pActor->GetEntity()->GetScriptTable(), isReset, m_pGameFramework->IsChannelOnHold(channelId));
	}
}

// -------------------------------------
void SC_Server::OnClientDisconnect(IGameRules* pGR, int channelId, int cause, const char* desc)
{
	CActor* pActor = g_pGame->GetGameRules()->GetActorByChannelId(channelId);
	if (!pActor)
	{
		GetEvents()->Call(SERVER_SCRIPT_EVENT_OnChannelDisconnect, channelId, (int)cause, desc);
	}
	else if (IEntity * pEntity = pActor->GetEntity())
	{
		GetEvents()->Call(SERVER_SCRIPT_EVENT_OnClientDisconnect, channelId, pEntity->GetScriptTable(), desc);
	}
}

// -------------------------------------
bool SC_Server::OnChatMessageEx(SmartScriptTable& Modifications, EChatMessageType type, EntityId sourceId, EntityId targetId, const char* msg, bool SvIsFake)
{
	// SC Fixme
	int SvCustomTeam = -1;
	return GetEvents()->Get(SERVER_SCRIPT_EVENT_OnChatMessage, Modifications, ScriptHandle(sourceId), ScriptHandle(targetId), msg, int(type), SvCustomTeam, SvIsFake);
}

// -------------------------------------
void SC_Server::OnVehicleInit(IEntity *pVehicle)
{
	GetVehicleScriptBind()->AttachTo(pVehicle);
}

// -------------------------------------
void SC_Server::OnGunTurretInit(IEntity *pGunTurret)
{
	GetGunTurretScriptBind()->AttachTo(pGunTurret);
}

// -------------------------------------
void SC_Server::OnWeaponInit(CWeapon *pWeapon)
{
	pWeapon->AddEventListener(GetEvents(), "ServerEvents");
}

// -------------------------------------
bool SC_Server::CanAttachAccessory(IScriptTable* pOwner, IScriptTable* pItem, const char* accessoryName)
{
	bool Ok = true;
	if (!GetEvents()->Get(SERVER_SCRIPT_EVENT_OnSwitchAccessory, Ok, pItem, pOwner, accessoryName))
	{
		return true; // fallback
	}

	return Ok;
}

// -------------------------------------
void SC_Server::OnEnterWeaponModify(IScriptTable* pOwner, IScriptTable* pItem)
{
	GetEvents()->Call(SERVER_SCRIPT_EVENT_OnEnterWeaponModify, pOwner, pItem, pOwner);
}

// -------------------------------------
void SC_Server::OnLeaveWeaponModify(IScriptTable* pOwner, IScriptTable* pItem)
{
	GetEvents()->Call(SERVER_SCRIPT_EVENT_OnLeaveWeaponModify, pOwner, pItem, pOwner);
}

// -------------------------------------
void SC_Server::OnWeaponStartFire(IScriptTable* pOwner, IScriptTable* pWeapon, EntityId ammoId, const char* ammoClass, Vec3 Pos, Vec3 Hit, Vec3 Dir)
{
	GetEvents()->Call(SERVER_SCRIPT_EVENT_OnShoot, pOwner, pWeapon, ScriptHandle(ammoId), ammoClass, Pos, Hit, Dir);
}

// -------------------------------------
void SC_Server::OnPlayerExplosiveDestroyed(EntityId playerId, EntityId explosiveId, int typeId, int numRemaining, bool HasExploded)
{
	GetEvents()->Call(SERVER_SCRIPT_EVENT_OnExplosiveRemoved, ScriptHandle(playerId), ScriptHandle(explosiveId), typeId, numRemaining, HasExploded);
}

// -------------------------------------
void SC_Server::OnPlayerExplosivePlaced(EntityId playerId, EntityId explosiveId, int typeId, int numRemaining, int numLimit)
{
	GetEvents()->Call(SERVER_SCRIPT_EVENT_OnExplosivePlaced, ScriptHandle(playerId), ScriptHandle(explosiveId), typeId, numRemaining, numLimit);
}

// -------------------------------------
void SC_Server::OnPlayerRequestHitAssistance(EntityId playerId, bool enable)
{
	GetEvents()->Call(SERVER_SCRIPT_EVENT_OnHitAssistance, ScriptHandle(playerId), enable);
}

// -------------------------------------
void SC_Server::OnRadarScanComplete(EntityId ownerId, EntityId weaponId, float radius)
{
	GetEvents()->Call(SERVER_SCRIPT_EVENT_OnRadarScanComplete, ScriptHandle(ownerId), ScriptHandle(weaponId), radius);
}

// -------------------------------------
bool SC_Server::CheckRocketCollision(CRocket* pRocket, EntityId ownerId, IEntity* pCollidee)
{
	if (g_pGameCVars->server_IgnoreOwnerProjectileCollisions)
	{
		bool Ignore = false;
		EntityId pTargetId = pCollidee->GetId();

		// We don't collide with our owners entity (or vehicle)
		if (ownerId == pTargetId)
		{
			Ignore = true;
		}
		else if (IEntity* pOwner = gEnv->pEntitySystem->GetEntity(ownerId))
		{
			if (IActor* pOwnerActor = gEnv->pGame->GetIGameFramework()->GetIActorSystem()->GetActor(ownerId))
			{
				if (IVehicle* pVehicle = pOwnerActor->GetLinkedVehicle())
				{
					if (pVehicle->GetEntityId() == pTargetId)
					{
						Ignore = true;
					}
				}
			}
		}

		if (Ignore)
		{
			float IgnoreTimeout = g_pGameCVars->server_IgnoreOwnerProjectileCollisionsTimeout;
			if (IgnoreTimeout <= 0.f || (gEnv->pTimer->GetCurrTime() - pRocket->m_LaunchTime <= IgnoreTimeout))
			{
				return false;
			}
		}
	}
	return true;
}

// -------------------------------------
bool SC_Server::CheckProjectileHit(EntityId shooterId, EntityId projectileId, bool destroyed, float damage, EntityId weaponId, Vec3 Pos, Vec3 Normal)
{
	bool HitOk = true;
	GetEvents()->Get(SERVER_SCRIPT_EVENT_OnProjectileHit, HitOk, ScriptHandle(shooterId), ScriptHandle(projectileId), destroyed, damage, ScriptHandle(weaponId), Pos, Normal);
	return HitOk;
}

// -------------------------------------
bool SC_Server::CanStartNextLevel()
{
	bool Proceed = false;
	if (GetEvents()->Get(SERVER_SCRIPT_EVENT_CanStartNextLevel, Proceed) && !Proceed)
	{
		return false;
	}
	return Proceed;
}

// -------------------------------------
bool SC_Server::OnActorDropItem(EntityId actorId, EntityId itemId)
{
	bool CanDropItem = true;
	if (GetEvents()->Get(SERVER_SCRIPT_EVENT_RequestDropWeapon, CanDropItem, ScriptHandle(actorId), ScriptHandle(itemId)) && !CanDropItem)
	{
		return false;
	}
	return true;
}

// -------------------------------------
bool SC_Server::CanActorPickUpItem(EntityId actorId, EntityId itemId, bool isObjectGrab)
{
	bool CanPickUp = true;
	if (isObjectGrab)
	{
		if (GetEvents()->Get(SERVER_SCRIPT_EVENT_RequestPickObject, CanPickUp, ScriptHandle(actorId), ScriptHandle(itemId)) && !CanPickUp)
		{
			return false;
		}
	}
	else if (GetEvents()->Get(SERVER_SCRIPT_EVENT_RequestPickWeapon, CanPickUp, ScriptHandle(actorId), ScriptHandle(itemId)) && !CanPickUp)
	{
		return false;
	}
	return true;
}

// -------------------------------------
bool SC_Server::CanActorUseItem(EntityId actorId, EntityId itemId)
{
	bool CanUse = true;
	if (GetEvents()->Get(SERVER_SCRIPT_EVENT_RequestUseWeapon, CanUse, ScriptHandle(actorId), ScriptHandle(itemId)) && !CanUse)
	{
		return false;
	}
	return true;
}

// -------------------------------------
void SC_Server::OnWallJump(EntityId ownerId, EntityId weaponId)
{
	GetEvents()->Call(SERVER_SCRIPT_EVENT_OnWallJump, ownerId ? ScriptHandle(ownerId) : 0, ScriptHandle(weaponId));
}

// -------------------------------------
void SC_Server::LogDebug(const char* format, ...) {
	va_list args;
	va_start(args, format);
	SC_ServerLog::Log("[server-debug] ", format, args);
	va_end(args);
}

// -------------------------------------
void SC_Server::Log(const char* format, ...) {
	va_list args;
	va_start(args, format);
	SC_ServerLog::Log("$9[$4CryMP-Server$9] ", format, args);
	va_end(args);
}

// -------------------------------------
void SC_Server::LogError(const char* format, ...) {
	va_list args;
	va_start(args, format);
	SC_ServerLog::Log("$9[$4CryMP-Server$9] Error: ", format, args);
	va_end(args);
}

// -------------------------------------
void SC_Server::LogWarning(const char* format, ...) {
	va_list args;
	va_start(args, format);
	SC_ServerLog::Log("$9[$4CryMP-Server$9] Warning: ", format, args);
	va_end(args);
}