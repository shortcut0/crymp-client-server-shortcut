// ???
#include "CryCommon/CrySystem/ISystem.h"
#include "CryCommon/CryEntitySystem/IEntity.h"

// Game
#include "CryGame/Items/Weapons/Weapon.h"
#include "CryGame/GameRules.h"
#include "CryGame/Items/Weapons/WeaponSystem.h"
#include "CryGame/Items/Weapons/Projectile.h"

// Server
#include "SC_ServerEvents.h"
#include "SC_Server.h"

// --------------------------------------------------------
SC_ServerEvents::SC_ServerEvents()
{
	m_pSS = gEnv->pScriptSystem;
}

// --------------------------------------------------------
SC_ServerEvents::~SC_ServerEvents()
{
	for (HSCRIPTFUNCTION handler : m_handlers)
	{
		if (handler)
			m_pSS->ReleaseFunc(handler);
	}
}


// --------------------------------------------------------
std::pair<SmartScriptTable, SmartScriptFunction> SC_ServerEvents::GetFunc(const std::string& handle)
{

	HSCRIPTFUNCTION pFunc_r = nullptr;
	SmartScriptTable pHost = nullptr;

	// ------------
	const std::string delims(".");
	const std::vector<std::string> tokens = SplitString(handle, *delims.c_str());

	int c(0);
	for (const auto& token : tokens) {

		c++; // crazy

		if (c >= tokens.size()) {
			if (c == 1) {
				if (!m_pSS->GetGlobalValue(token.c_str(), pFunc_r)) {
					gSC_Server->LogError("SC_ServerEvents::GetFunc Function %s Not found at index %d, Input Host was %s", token.c_str(), c, handle.c_str());
				}
			}
			else
			{
				if (pHost && !pHost->GetValue(token.c_str(), pFunc_r)) {
					gSC_Server->LogError("SC_ServerEvents::GetFunc Function %s Not found at index %d, Input Host was %s", token.c_str(), c, handle.c_str());
				}
			}
			break;
		}

		if (c == 1) {
			m_pSS->GetGlobalValue(token.c_str(), pHost);
			if (!pHost) {
				gSC_Server->LogError("SC_ServerEvents::GetFunc Global Host %s not found", token.c_str());
				break;
			}
			continue;
		}

		if (pHost && !pHost->GetValue(token.c_str(), pHost)) {
			pHost = nullptr;
			//pFunc = nullptr;
			gSC_Server->LogError("SC_ServerEvents::GetFunc Host Array %s not found at index %d, Input Host was %s", token.c_str(), c, handle.c_str());
			break;
		}
	}

	//if (pFunc_r != nullptr)
	//	pFunc = pFunc_r;

	//SmartScriptFunction smartFunc(m_pSS, pFunc_r);
	//return std::make_pair(pHost, smartFunc);
	return std::make_pair(pHost, SmartScriptFunction(m_pSS, pFunc_r));
}


// --------------------------------------------------------
bool SC_ServerEvents::SetHandler(EServerEvents callback, HSCRIPTFUNCTION handler)
{
	if (callback < 0 || callback >= SV_EVENT_COUNT)
	{
		// invalid callback
		return false;
	}

	if (m_handlers[callback])
	{
		// handlers cannot be replaced
		return false;
	}

	m_handlers[callback] = handler;

	return true;
}

// --------------------------------------------------------
void SC_ServerEvents::OnUpdate(float deltaTime)
{
	Call(SV_EVENT_ON_UPDATE, deltaTime);
}

// --------------------------------------------------------
void SC_ServerEvents::OnSpawn(IEntity* pEntity)
{
	IScriptTable* pScript = pEntity->GetScriptTable();
	if (pScript)
	{
		Call(SV_EVENT_ON_SPAWN, pScript);
	}
}

// --------------------------------------------------------
void SC_ServerEvents::OnGameRulesCreated(EntityId gameRulesId)
{
	ScriptHandle id;
	id.n = gameRulesId;
	Call(SV_EVENT_ON_GAME_RULES_CREATED, id);
}

// --------------------------------------------------------
// --------------------------------------------------------
// --------------------------------------------------------
// Weapon Listener
void SC_ServerEvents::OnShoot(IWeapon* pWeapon, EntityId shooterId, EntityId ammoId, IEntityClass* pAmmoType, const Vec3& pos, const Vec3& dir, const Vec3& vel) {

}



void SC_ServerEvents::OnStartFire(IWeapon* pWeapon, EntityId shooterId) {

}


void SC_ServerEvents::OnStopFire(IWeapon* pWeapon, EntityId shooterId) {

}


void SC_ServerEvents::OnStartReload(IWeapon* pWeapon, EntityId shooterId, IEntityClass* pAmmoType) {
	if (CWeapon* pWpn = static_cast<CWeapon*>(pWeapon)) {
		EntityId weaponId = pWpn->GetEntityId();
		Call(SERVER_SCRIPT_EVENT_OnStartReload, ScriptHandle(shooterId), ScriptHandle(weaponId));
	}
}


void SC_ServerEvents::OnEndReload(IWeapon* pWeapon, EntityId shooterId, IEntityClass* pAmmoType) {
	if (CWeapon* pWpn = static_cast<CWeapon*>(pWeapon)) {
		EntityId weaponId = pWpn->GetEntityId();
		Call(SERVER_SCRIPT_EVENT_OnEndReload, ScriptHandle(shooterId), ScriptHandle(weaponId));
	}

}


void SC_ServerEvents::OnOutOfAmmo(IWeapon* pWeapon, IEntityClass* pAmmoType) {

}


void SC_ServerEvents::OnReadyToFire(IWeapon* pWeapon) {

}


void SC_ServerEvents::OnPickedUp(IWeapon* pWeapon, EntityId actorId, bool destroyed) {

}


void SC_ServerEvents::OnDropped(IWeapon* pWeapon, EntityId actorId) {

}


void SC_ServerEvents::OnMelee(IWeapon* pWeapon, EntityId shooterId) {

	if (CWeapon* pWpn = static_cast<CWeapon*>(pWeapon)) {
		EntityId weaponId = pWpn->GetEntityId();
		Call(SERVER_SCRIPT_EVENT_OnMelee, ScriptHandle(shooterId), ScriptHandle(weaponId));
	}
}


void SC_ServerEvents::OnStartTargetting(IWeapon* pWeapon) {

}


void SC_ServerEvents::OnStopTargetting(IWeapon* pWeapon) {

}


void SC_ServerEvents::OnSelected(IWeapon* pWeapon, bool selected) {

}