// ===================================================================================
//          ____            __  __ ____            ____                             --
//         / ___|_ __ _   _|  \/  |  _ \          / ___|  ___ _ ____   _____ _ __   --
//        | |   | '__| | | | |\/| | |_) |  _____  \___ \ / _ \ '__\ \ / / _ \ '__|  --
//        | |___| |  | |_| | |  | |  __/  |_____|  ___) |  __/ |   \ V /  __/ |     --
//         \____|_|   \__, |_|  |_|_|             |____/ \___|_|    \_/ \___|_|     --
//                    |___/          by: shortcut0                                  --
// ===================================================================================

#pragma once

#ifndef __SC_SERVER_H__
#define __SC_SERVER_H__

// Windows
#include <memory>
#include <string>
#include <string_view>
#include <vector>
#include <filesystem>

#include "Library/WinAPI.h"

// Listeners
#include "CryCommon/CryAction/IGameFramework.h"
#include "CryCommon/CryAction/ILevelSystem.h"
#include "CryCommon/CryEntitySystem/IEntitySystem.h"
#include "CryCommon/CryScriptSystem/IScriptSystem.h"

// Utils
#include "Utils\SC_FileSaver.h"
#include "Utils\SC_ServerStats.h"
#include "Utils\SC_AntiCheat.h"
#include "Utils\SC_LuaFileSystem.h"
#include "Utils\SC_ServerUtils.h"
#include "Utils\SC_ServerTimer.h"
#include "Utils\SC_ServerAStar.h"

// Game extensions
#include "Extensions\ScriptBind_SC_Server.h"
#include "Extensions\ScriptBind_SC_Vehicle.h"
#include "Extensions\ScriptBind_SC_GunTurret.h"

// Mains?
#include "SC_ServerCVars.h"
#include "SC_ServerEvents.h"
//#include "SC_ServerPublisher.h" // TODO: Delete, implemented in LUA (although, i would prefer a C++ version. whith it's very own Script Bind!!)


#include "CryMP/Server/SSM.h"
#include "CryMP/Common/HTTPClient.h"
#include "CryCommon/CryNetwork/INetworkService.h"
#include "CryScriptSystem/ScriptSystem.h"

class Executor;
class HTTPClient;
//class CRocket;
//struct ExplosionInfo; // for debug explosions



// ---------------------------------
class SC_Server : public ISSM, public IGameFrameworkListener, public ILevelSystemListener, public IEntitySystemSink
{

	//IGameFramework* m_pGameFramework = nullptr;
	IGame* m_pGame = nullptr;

	// ------------------------------
	static void m_pExitHandler();

	// ------------------------------
	IGameFramework* m_pGameFramework = nullptr;
	INetworkService* m_pGSMaster = nullptr;

	// ------------------------------
	std::unique_ptr<Executor> m_pExecutor;
	std::unique_ptr<HTTPClient> m_pHttpClient;
	//std::unique_ptr<ServerPublisher> m_pServerPublisher;
	std::unique_ptr<SC_ServerEvents> m_pEventsCallback;
	std::unique_ptr<ScriptBind_SC_Server> m_pScriptBindServer;
	std::unique_ptr<SC_LuaFileSystem> m_pLuaFileSystem;
	std::unique_ptr<SC_ServerUtils> m_pServerUtils;
	std::unique_ptr<SC_ServerStats> m_pServerStats;
	std::unique_ptr<SC_ServerAnticheat> m_pAC;
	std::unique_ptr<SC_ServerTimer> m_pServerTimer;
	std::unique_ptr<CScriptBind_SC_ServerVehicle> m_pServerVehicleScriptBind;
	std::unique_ptr<CScriptBind_SC_GunTurret> m_pServerGunTurretScriptBind;
	std::unique_ptr<SC_AStar> m_pAStar;
	std::unique_ptr<SC_ServerCVars> m_pCVars;
	std::unique_ptr<SC_ServerFileSaver> m_pFileSaver;

	// ------------------------------

	std::vector<std::pair<EntityId, float>> m_scheduledEntityRemovals;
	void UpdateEntityRemoval();
	//void ScheduleEntityRemoval();

	std::vector<std::string> m_masters;

	std::filesystem::path m_rootDir;
	std::filesystem::path m_workingDir;

	// ------------------------------


	// ------------------------------

	void InitGameOverwrites(const std::string& filename);
	std::vector<std::pair<std::string, std::string>> m_pOverwriteSF;

	// ------------------------------

	IScriptSystem* m_pSS = nullptr;
	std::string m_ScriptPath;
	std::string m_ServerFolder = "CryMP-Server-Light"; // FIXME

public:

	// ------------------------------
	SC_Server(IGameFramework* pFW, ISystem* pSystem);
	~SC_Server();

	// ---------------------------------
	void Quit() ;


	// ============== LOGS =============
	void Log(const char* format, ...); //
	void LogError(const char* format, ...); // Error:
	void LogWarning(const char* format, ...); // Warning:
	void LogDebug(const char* format, ...); // [server-debug]

	// ============== INIT =============
	void Init(IGameFramework* pGameFramework);
	void OnGameStart(IGameObject* pGameRules);
	void Update(float dT);

	// ============== SCRIPTS =============

	SC_ServerTimer m_ScriptFrameTimer;
	SC_ServerTimer m_ScriptSecondTimer;
	SC_ServerTimer m_ScriptMinuteTimer;
	SC_ServerTimer m_ScriptHourTimer;

	void QuitScript();
	void InitScript(bool PostInitialize = true);
	void RegisterScriptGlobals();
	void ScriptInitEntities();
	void ScriptInitGlobals();
	void LoadScriptConfig();
	void LoadScript(bool ForceInit = false);
	void OnScriptError(const std::string& error);
	void OnScriptLoaded(const std::string& fileName, bool success);
	void OnLoadingScript(const std::string& fileName);
	bool OverwriteScriptPath(std::string &output, const std::string& input);

	bool IsLuaReady() const
	{
		bool error = false;
		if (m_pSS)
		{
			if (m_pSS->GetGlobalValue("SCRIPT_ERROR", error) && error)
			{
				return false;
			}
		}

		return (m_ScriptLoaded && m_ScriptInitialized);
	}

	bool m_ScriptLogErrors = true;
	bool m_ScriptInitialized = false;
	bool m_ScriptLoaded = false;
	bool m_ScriptQuitting = false;
	bool m_ScriptPostInitialized = false;
	bool m_UseAutomaticScriptRedirection = false;


	// ------------------------------
	// ISSM.h

	void OnGameRulesLoad(IGameRules* pGR) {};
	void OnGameRulesUnload(IGameRules* pGR) {};

	void OnClientEnteredGame(IGameRules* pGR, int channelId, bool isReset);
	void OnClientConnect(IGameRules* pGR, int channelId, bool isReset);
	void OnClientDisconnect(IGameRules* pGR, int channelId, int cause, const char* desc);
	bool CanStartNextLevel();
	bool CanReceiveChatMessage(EChatMessageType type, EntityId sourceId, EntityId targetId)
	{
		// SC Fixme: CVar
		return true;
	}

	std::optional<std::string> OnChatMessage(IGameRules* pGR, EChatMessageType type, EntityId sourceId, EntityId targetId, const std::string& msg) { return msg; }; // we have our own handler
	std::optional<std::string> OnPlayerRename(IGameRules* pGR, IActor* pActor, const std::string& name) { return name; }; //TODO
	bool IsRMILegitimate(INetChannel* pNC, EntityId sourceId)
	{
		return true;// TODO
	};
	bool IsHitRMILegitimate(INetChannel* pNC, EntityId sourceId, EntityId weaponId)
	{
		return true;// TODO
	};
	bool OnChatMessageEx(SmartScriptTable &Modifications, EChatMessageType type, EntityId sourceId, EntityId targetId, const char* msg, bool SvIsFake);
	void OnVehicleInit(IEntity* pVehicle);
	void OnGunTurretInit(IEntity* pGunTurret);
	void OnWeaponInit(CWeapon* pWeapon);
	bool CanAttachAccessory(IScriptTable* pOwner, IScriptTable* pItem, const char* accessoryName);
	void OnEnterWeaponModify(IScriptTable* pOwner, IScriptTable* pItem);
	void OnLeaveWeaponModify(IScriptTable* pOwner, IScriptTable* pItem);
	void OnWeaponStartFire(IScriptTable* pOwner, IScriptTable* pWeapon, EntityId ammoId, const char* ammoClass, Vec3 Pos, Vec3 Hit, Vec3 Dir);
	void OnPlayerExplosiveDestroyed(EntityId playerId, EntityId explosiveId, int typeId, int numRemaining, bool HasExploded);
	void OnPlayerExplosivePlaced(EntityId playerId, EntityId explosiveId, int typeId, int numRemaining, int numLimit);
	void OnPlayerRequestHitAssistance(EntityId playerId, bool enable);
	bool CheckRocketCollision(CRocket* pRocket, EntityId ownerId, IEntity* pCollidee);
	void OnRadarScanComplete(EntityId ownerId, EntityId weaponId, float radius);
	bool CheckProjectileHit(EntityId shooterId, EntityId projectileId, bool destroyed, float damage, EntityId weaponId, Vec3 Pos, Vec3 Normal);
	bool ProcessServerExplosion(ExplosionInfo& OverwriteInfo)
	{
		// TODO
		return true;
	}

	bool LoadAutoexec()
	{
		return false; // We have our own handler!
	}
	bool LogScriptErrors()
	{
		return m_ScriptLogErrors;
	}
	bool OnActorDropItem(EntityId ownerId, EntityId itemId);
	bool CanActorPickUpItem(EntityId ownerId, EntityId itemId, bool isObjectGrab);
	bool CanActorUseItem(EntityId ownerId, EntityId itemId);
	void OnWallJump(EntityId ownerId, EntityId weaponId);
	// ------------------------------

	enum class PostInitEntityTypes {
		ePIT_None = 0,
		ePIT_Item,
		ePIT_Vehicle,
		ePIT_Actor,
		ePIT_End, // not needed nor used, but must be last.. i guess ee
	};

	std::vector<std::pair<EntityId, PostInitEntityTypes>> m_PreSpawnedEntities; // spawned before script initialized, so collect here and call later
	EntityId m_LastSpawnId = EntityId(0);
	int m_SpawnCounter = 0;

	EntityId GetLastSpawnId() const { return m_LastSpawnId; }
	IEntity* GetLastSpawnEntity() { return gEnv->pEntitySystem->GetEntity(m_LastSpawnId); }

	// ------------------------------

	int m_GenericCounter = 0;
	int GetCounter() { m_GenericCounter++; return m_GenericCounter; };

	/*
	std::vector<std::string> m_uniqueNames;
	const char* GetUniqueName(const char* name) {
		int c=GetCounter();
		m_uniqueNames.push_back(std::string(name)+"_"+std::to_string(c));
		return m_uniqueNames[m_uniqueNames.size()-1].c_str();
	};*/

	// ------------------------------
	int test_cvar_;
	void InitCVars(IConsole* pConsole);
	void InitCommands(IConsole* pConsole);
	static void OnInitLuaCmd(IConsoleCmdArgs* pArgs);

	// ------------------------------
	void InitMasters();
	void HttpGet(const std::string_view& url, std::function<void(HTTPClientResult&)> callback);
	void HttpRequest(HTTPClientRequest&& request);
	std::string GetMasterServerAPI(const std::string& master);
	INetworkService* GetGSMaster() const { return m_pGSMaster; }

	const std::vector<std::string>& GetMasters() const {
		return m_masters;
	}

	// ------------------------------
	IGameFramework* GetGameFramework() { return m_pGameFramework; }
	Executor* GetExecutor() { return m_pExecutor.get(); }
	HTTPClient* GetHTTPClient() { return m_pHttpClient.get(); }

	CScriptBind_SC_ServerVehicle* GetVehicleScriptBind() const { return m_pServerVehicleScriptBind.get(); };
	CScriptBind_SC_GunTurret* GetGunTurretScriptBind() const { return m_pServerGunTurretScriptBind.get(); };

	// -----------------------------
	SC_AStar* GetAStar() const { return m_pAStar.get(); }
	template<class... Params>
	int AStar_Path(const Params &... params) {

		// ===================================
		// Call Limiter, discards calls exceeding limit
		if (m_AStarTimer.Expired_Refresh(GetCVars()->astar_callsLimitTime))
		{
			m_AStarCalls = 0;
		}

		m_AStarCalls++;
		if (m_AStarCalls >= GetCVars()->astar_callsLimit)
		{
			Log("skip.");
			return 0;
		}

		return GetAStar()->FindPath(params...);
	}

	SC_ServerEvents* GetEvents() const { return m_pEventsCallback.get(); }
	SC_ServerUtils* GetUtils() const { return m_pServerUtils.get(); }
	SC_ServerAnticheat* GetAC() const { return m_pAC.get(); }
	SC_ServerStats* GetStats() const { return m_pServerStats.get(); }
	SC_LuaFileSystem* GetLFS() const { return m_pLuaFileSystem.get(); }
	SC_ServerTimer* GetTimer() const { return m_pServerTimer.get(); }
	SC_ServerCVars* GetCVars() const { return m_pCVars.get(); }
	SC_ServerFileSaver* GetFileSaver() const { return m_pFileSaver.get(); }
	std::string GetRoot() const { return m_rootDir.string(); }
	std::string GetWorkingDir() const { return m_workingDir.string(); }


	// ------------------------------
	enum class EGameSpyUpdateType {
		eGSUpdate_Server = 0,
		eGSUpdate_Player,
		eGSUpdate_Team
	};

	bool UpdateGameSpyServerReport(EGameSpyUpdateType type, const char* key, const char* value, int index = 0);

private:
	// IGameFrameworkListener
	void OnPostUpdate(float deltaTime) override;
	void OnSaveGame(ISaveGame* saveGame) override;
	void OnLoadGame(ILoadGame* loadGame) override;
	void OnLevelEnd(const char* nextLevel) override;
	void OnActionEvent(const SActionEvent& event) override;

	// ILevelSystemListener
	void OnLevelNotFound(const char* levelName) override;
	void OnLoadingStart(ILevelInfo* pLevel) override;
	void OnLoadingComplete(ILevel* pLevel) override;
	void OnLoadingError(ILevelInfo* pLevel, const char* error) override;
	void OnLoadingProgress(ILevelInfo* pLevel, int progressAmount) override;

	// IEntitySystemSink
	bool OnBeforeSpawn(SEntitySpawnParams& params) override;
	void OnSpawn(IEntity* pEntity, SEntitySpawnParams& params) override;
	bool OnRemove(IEntity* pEntity) override;
	void OnEvent(IEntity* pEntity, SEntityEvent& event) override;

	void SetCVars();
	bool ReadCfg(const std::string& filepath);

	//
	bool m_UseOnlyHTTP = false;
	bool m_Initialized = false;
	float m_tickCounter = 0;
	float m_tickGoal = 0;
	float m_minGoal = 0;
	float m_hourGoal = 0;

	float m_PreviousRate;
	float m_IdleRate;
	bool m_IsIdle;
	ICVar* m_MaxRateCVar;

	// Stats
	SC_ServerTimer m_StatsTimer;

	// Game
	SC_ServerTimer m_ExplosionTimer;
	int m_ProcessedExplosions = 0;

	// Stuff
	SC_ServerTimer m_AStarTimer;
	int m_AStarCalls = 0;

	// Debug
	SC_ServerTimer m_DebugTimer3;
	SC_ServerTimer m_DebugTimer2;
	SC_ServerTimer m_DebugTimer;
	void Debug();

public:

	std::vector<int> m_GameChannels;
	int m_lastChannel = 0;

	bool ProcessExplosion(const ExplosionInfo& info);
	void DebugExplosion(const ExplosionInfo& info);
	void SpawnDebugParticle(Vec3 pos = Vec3(ZERO), Vec3 dir = Vec3(0, 0, 1), float scale = 1.f);
};

///////////////////////
inline SC_Server* gSC_Server;
///////////////////////

#endif // __SERVER_H__
