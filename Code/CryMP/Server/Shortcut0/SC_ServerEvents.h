#pragma once
/*
#include <map>
#include <vector>*/

#include <regex>
#include <array>

#include <iostream>
#include <sstream>  // Include this header for stringstream
#include <vector>
#include <string>
#include <map>

//#include "CryCommon/CryAction/IWeapon.h"
#include "CryGame/Items/Weapons/Weapon.h"
#include "CryCommon/CryScriptSystem/IScriptSystem.h"


struct IEntity;
enum EServerEvents
{
	SV_EVENT_ON_UPDATE,
	SV_EVENT_ON_DISCONNECT,
	SV_EVENT_ON_SPAWN,
	SV_EVENT_ON_LOADING_START,
	SV_EVENT_ON_GAME_RULES_CREATED,

	// must be last
	SV_EVENT_COUNT
};

enum EServerScriptEvents
{
	SERVER_SCRIPT_EVENT_OnCheat,
	SERVER_SCRIPT_EVENT_RequestDropWeapon,
	SERVER_SCRIPT_EVENT_RequestPickWeapon,
	SERVER_SCRIPT_EVENT_RequestUseWeapon,
	SERVER_SCRIPT_EVENT_OnWeaponDropped,
	SERVER_SCRIPT_EVENT_OnShoot,
	SERVER_SCRIPT_EVENT_OnStartReload,
	SERVER_SCRIPT_EVENT_OnEndReload,
	SERVER_SCRIPT_EVENT_OnMelee,
	SERVER_SCRIPT_EVENT_RequestPickObject,
	SERVER_SCRIPT_EVENT_OnObjectPicked,
	SERVER_SCRIPT_EVENT_OnExplosivePlaced,
	SERVER_SCRIPT_EVENT_OnExplosiveRemoved,
	SERVER_SCRIPT_EVENT_OnHitAssistance,
	SERVER_SCRIPT_EVENT_OnConnection,
	SERVER_SCRIPT_EVENT_OnChannelDisconnect,
	SERVER_SCRIPT_EVENT_OnClientDisconnect,
	SERVER_SCRIPT_EVENT_OnClientEnteredGame,
	SERVER_SCRIPT_EVENT_OnWallJump,
	SERVER_SCRIPT_EVENT_OnChatMessage,
	SERVER_SCRIPT_EVENT_OnEntityCollision,
	SERVER_SCRIPT_EVENT_OnSwitchAccessory,
	SERVER_SCRIPT_EVENT_OnProjectileHit,
	SERVER_SCRIPT_EVENT_OnLeaveWeaponModify,
	SERVER_SCRIPT_EVENT_OnProjectileExplosion,
	SERVER_SCRIPT_EVENT_CanStartNextLevel,
	SERVER_SCRIPT_EVENT_OnRadarScanComplete,
	SERVER_SCRIPT_EVENT_OnUpdate,
	SERVER_SCRIPT_EVENT_OnTimer,
	SERVER_SCRIPT_EVENT_OnGameShutdown,
	SERVER_SCRIPT_EVENT_OnMapStarted,
	SERVER_SCRIPT_EVENT_OnEntitySpawn,
	SERVER_SCRIPT_EVENT_OnVehicleSpawn,
	SERVER_SCRIPT_EVENT_OnScriptLoaded,
	SERVER_SCRIPT_EVENT_OnMapCommand,
	SERVER_SCRIPT_EVENT_OnScriptError,
	SERVER_SCRIPT_EVENT_OnCVarChanged,
	SERVER_SCRIPT_EVENT_OnClientConnect,
	SERVER_SCRIPT_EVENT_OnLoadingScript,
	SERVER_SCRIPT_EVENT_OnLevelStart,
	SERVER_SCRIPT_EVENT_OnEnterWeaponModify,

	// must be lvst
	SERVER_SCRIPT_EVENT_MAX
};

class SC_ServerEvents : public IWeaponEventListener
{
	IScriptSystem* m_pSS = nullptr;
	std::array<HSCRIPTFUNCTION, SV_EVENT_COUNT> m_handlers = {};

	template<class... Params>
	void Call(EServerEvents callback, const Params &... params)
	{
		HSCRIPTFUNCTION handler = m_handlers[callback];

		if (handler && m_pSS->BeginCall(handler))
		{
			(m_pSS->PushFuncParam(params), ...);
			m_pSS->EndCall();
		}
	}

	// --------------------
	std::pair<SmartScriptTable, SmartScriptFunction> GetFunc(const std::string& host);
	//bool GetFuncEx(const std::string& host, SmartScriptFunction& outFunc, SmartScriptTable& outTable);
	
private:

	// --------------------
	bool Regexp(const std::string& itemName, const std::string& filter) {
		std::regex pattern(filter);
		return std::regex_match(itemName, pattern);
	}

	// --------------------
	std::vector<std::string> SplitString(const std::string& str, char delimiter) {
		std::vector<std::string> tokens;
		std::stringstream ss(str);
		std::string token;

		while (std::getline(ss, token, delimiter)) {
			tokens.push_back(token);
		}

		return tokens;
	}

	const std::string ScriptEvents[SERVER_SCRIPT_EVENT_MAX] =
	{
		"OnCheat",
		"RequestDropWeapon",
		"RequestPickWeapon",
		"RequestUseWeapon",
		"OnWeaponDropped",
		"OnShoot",
		"OnStartReload",
		"OnEndReload",
		"OnMelee",
		"RequestPickObject",
		"OnObjectPicked",
		"OnExplosivePlaced",
		"OnExplosiveRemoved",
		"OnHitAssistance",
		"OnConnection",
		"OnChannelDisconnect",
		"OnClientDisconnect",
		"OnClientEnteredGame",
		"OnWallJump",
		"OnChatMessage",
		"OnEntityCollision",
		"OnSwitchAccessory",
		"OnProjectileHit",
		"OnLeaveWeaponModify",
		"OnProjectileExplosion",
		"CanStartNextLevel",
		"OnRadarScanComplete",
		"OnUpdate",
		"OnTimer",
		"OnGameShutdown",
		"OnMapStarted",
		"OnEntitySpawn",
		"OnVehicleSpawn",
		"OnScriptLoaded",
		"OnMapCommand",
		"OnScriptError",
		"OnCVarChanged",
		"OnClientConnect",
		"OnLoadingScript",
		"OnLevelStart",
		"OnEnterWeaponModify",
	};

public:

	SC_ServerEvents();
	~SC_ServerEvents();

	bool SetHandler(EServerEvents callback, HSCRIPTFUNCTION handler);

	void OnUpdate(float deltaTime);
	void OnSpawn(IEntity* pEntity);
	void OnGameRulesCreated(EntityId gameRulesId);


	// host for event calls
	std::string ScriptHost;

	// Client Stuff
	//void OnBecomeLocalActor(EntityId localActorId);
	//void OnLoadingStart();
	//void OnDisconnect(int reason, const char* message);
	//void OnMasterResolved();

	template<class... Params>
	bool Call(HSCRIPTFUNCTION handle, const Params &... params)
	{
		if (handle && m_pSS->BeginCall(handle))
		{
			(m_pSS->PushFuncParam(params), ...);
			m_pSS->EndCall();
		}
		else
			return false;

		return true;
	}

	template<class Ret, class... Params>
	bool Get(const EServerScriptEvents& handle, Ret& ret, const Params&... params)
	{
		if (handle < 0 || handle >= SERVER_SCRIPT_EVENT_MAX) // 
		{
			CryLogAlways("Invalid handle to Get(): %d", (int)handle);
			return false;
		}

		const std::string& Event = ScriptEvents[handle];
		return Get(ScriptHost + Event, ret, params...);
	}


	template<class Ret, class... Params>
	bool Get(const std::string& handle, Ret &ret, const Params &... params)
	{

		//std::pair<SmartScriptTable, SmartScriptFunction> map = GetFunc(handle);

		//SmartScriptFunction pFunc = std::move(map.second);
		//SmartScriptTable pHost = map.first;
		auto [pHost, pFunc] = GetFunc(handle);

		if (!pFunc) {
			//CryLogAlways("Get(): Function handle is nullptr (%s)", handle.c_str());
			return false;
		}

		bool bOk = true;
		if (pFunc && m_pSS->BeginCall(pFunc))
		{
			if (pHost)
				m_pSS->PushFuncParam(pHost); // push host (self in lua)

			(m_pSS->PushFuncParam(params), ...);
			m_pSS->EndCall(ret);
		}
		else bOk = false;

		return bOk;
	}

	template<class... Params>
	bool Call(const EServerScriptEvents& handle, const Params&... params)
	{
		if (handle < 0 || handle >= SERVER_SCRIPT_EVENT_MAX) // 
		{
			CryLogAlways("Invalid handle to Get(): %d", (int)handle);
			return false;
		}

		const std::string& Event = ScriptEvents[handle];
		return Call(ScriptHost + Event, params...);
	}

	template<class... Params>
	bool Call(const std::string& handle, const Params &... params)
	{
		auto [pHost, pFunc] = GetFunc(handle);

		if (!pFunc) {
			//CryLogAlways("Call(): Function handle is nullptr (%s)", handle.c_str());
			return false;
		}

		bool bOk = true;
		if (pFunc && m_pSS->BeginCall(pFunc))
		{
			if (pHost != nullptr)
				m_pSS->PushFuncParam(pHost); // push host (self in lua)

			(m_pSS->PushFuncParam(params), ...);
			m_pSS->EndCall();
		}
		else bOk = false;

		//m_pSS->ReleaseFunc(pFunc);
		return bOk;
	}

	private:

		// IWeaponListener
		virtual void OnShoot(IWeapon* pWeapon, EntityId shooterId, EntityId ammoId, IEntityClass* pAmmoType, const Vec3& pos, const Vec3& dir, const Vec3& vel) override;
		virtual void OnStartFire(IWeapon* pWeapon, EntityId shooterId) override;
		virtual void OnStopFire(IWeapon* pWeapon, EntityId shooterId) override;
		virtual void OnStartReload(IWeapon* pWeapon, EntityId shooterId, IEntityClass* pAmmoType) override;
		virtual void OnEndReload(IWeapon* pWeapon, EntityId shooterId, IEntityClass* pAmmoType) override;
		virtual void OnOutOfAmmo(IWeapon* pWeapon, IEntityClass* pAmmoType) override;
		virtual void OnReadyToFire(IWeapon* pWeapon) override;
		virtual void OnPickedUp(IWeapon* pWeapon, EntityId actorId, bool destroyed) override;
		virtual void OnDropped(IWeapon* pWeapon, EntityId actorId) override;
		virtual void OnMelee(IWeapon* pWeapon, EntityId shooterId) override;
		virtual void OnStartTargetting(IWeapon* pWeapon) override;
		virtual void OnStopTargetting(IWeapon* pWeapon) override;
		virtual void OnSelected(IWeapon* pWeapon, bool selected) override;
};
