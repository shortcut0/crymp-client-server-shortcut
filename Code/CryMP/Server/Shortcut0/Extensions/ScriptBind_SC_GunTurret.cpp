#include "CryCommon/CrySystem/ISystem.h"
#include "CryCommon/CryEntitySystem/IEntity.h"

#include "CryGame/Game.h"
#include "CryGame/Items/Weapons/GunTurret.h"
#include "CryGame/Items/Weapons/WeaponSystem.h"

#include "ScriptBind_SC_GunTurret.h"

//------------------------------------------------------------------------
CScriptBind_SC_GunTurret::CScriptBind_SC_GunTurret(ISystem* pSystem, IGameFramework* pGameFramework)
	: m_pSystem(pSystem),
	m_pSS(pSystem->GetIScriptSystem()),
	m_pGameFW(pGameFramework)
{
	Init(m_pSS, m_pSystem, 1);
	SetGlobalName("ServerGunTurret");

	RegisterMethods();
	RegisterGlobals();
}

//------------------------------------------------------------------------
CScriptBind_SC_GunTurret::~CScriptBind_SC_GunTurret()
{
}

//------------------------------------------------------------------------
void CScriptBind_SC_GunTurret::RegisterGlobals()
{
}

//------------------------------------------------------------------------
void CScriptBind_SC_GunTurret::RegisterMethods()
{
#undef SCRIPT_REG_CLASSNAME
#define SCRIPT_REG_CLASSNAME &CScriptBind_SC_GunTurret::

	SCRIPT_REG_TEMPLFUNC(SetTarget, "targetId, attackTime");
	SCRIPT_REG_TEMPLFUNC(SetAimTarget, "targetId, aimTime");
	SCRIPT_REG_TEMPLFUNC(SetAimPosition, "position, aimTime");
	SCRIPT_REG_TEMPLFUNC(SetLookAt, "yaw, pitch");
	SCRIPT_REG_TEMPLFUNC(StartFire, "secondary, fireTime");
	SCRIPT_REG_TEMPLFUNC(StopFire, "secondary");

	SCRIPT_REG_FUNC(StopFireAll);
	SCRIPT_REG_FUNC(ResetProperties);
	SCRIPT_REG_FUNC(ResetLookAt);

#undef SCRIPT_REG_CLASSNAME
}

//------------------------------------------------------------------------
void CScriptBind_SC_GunTurret::AttachTo(IEntity* pVehicle)
{
	IScriptTable* pScriptTable = pVehicle->GetScriptTable();
	if (pScriptTable)
	{
		SmartScriptTable thisTable(m_pSS);
		thisTable->SetValue("__this", ScriptHandle(pVehicle->GetId()));
		thisTable->Delegate(GetMethodsTable());

		pScriptTable->SetValue("GunTurret", thisTable);
	}

}

//------------------------------------------------------------------------
CWeapon* CScriptBind_SC_GunTurret::GetWeapon(IFunctionHandler *pH)
{
	void* pThis = pH->GetThis();

	if (pThis)
	{
		IItem* pItem = m_pGameFW->GetIItemSystem()->GetItem((EntityId)(UINT_PTR)pThis);
		if (pItem)
		{
			IWeapon* pWeapon = pItem->GetIWeapon();
			if (pWeapon)
				return static_cast<CWeapon*>(pWeapon);
		}
	}

	return 0;
}

//------------------------------------------------------------------------
CGunTurret* CScriptBind_SC_GunTurret::GetGunTurret(IFunctionHandler *pH)
{
	CWeapon* pWeapon = GetWeapon(pH);
	if (pWeapon)
	{
		if (CGunTurret* pGT = static_cast<CGunTurret*>(pWeapon))
		{
			return pGT;
		}
	}

	return 0;
}

//------------------------------------------------------------------------
int CScriptBind_SC_GunTurret::SetTarget(IFunctionHandler *pH, ScriptHandle targetId, float aimTime)
{
	CGunTurret* pTurret = GetGunTurret(pH);
	if (!pTurret)
	{
		return pH->EndFunction();
	}
	EntityId CTargetId(targetId.n);
	float CurrentTime = gEnv->pTimer->GetCurrTime();
	if (!CTargetId || aimTime <= 0) // Reset
	{ 
		pTurret->m_SC_FireTargetId = 0;
		pTurret->m_SC_WantTargetTime = CurrentTime;
	} 
	else
	{
		pTurret->m_SC_FireTargetId = CTargetId;
		pTurret->m_SC_WantTargetTime = CurrentTime + aimTime;
	}
	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_SC_GunTurret::SetAimTarget(IFunctionHandler *pH, ScriptHandle targetId, float aimTime)
{
	CGunTurret* pTurret = GetGunTurret(pH);
	if (!pTurret)
	{
		return pH->EndFunction();
	}

	EntityId CTargetId(targetId.n);
	float CurrentTime = gEnv->pTimer->GetCurrTime();
	if (!CTargetId || aimTime <= 0) // Reset
	{ 
		pTurret->m_SC_AimTargetId = 0;
		pTurret->m_SC_WantAimTime = CurrentTime;
	} 
	else
	{
		pTurret->m_SC_AimTargetId = CTargetId;
		pTurret->m_SC_WantAimTime = CurrentTime + aimTime;
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_SC_GunTurret::ResetProperties(IFunctionHandler *pH)
{
	CGunTurret* pTurret = GetGunTurret(pH);
	if (!pTurret)
	{
		return pH->EndFunction();
	}

	pTurret->SC_ResetProperties();
	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_SC_GunTurret::SetAimPosition(IFunctionHandler *pH, Vec3 Pos, float aimTime)
{
	CGunTurret* pTurret = GetGunTurret(pH);
	if (!pTurret)
	{
		return pH->EndFunction();
	}

	float CurrentTime = gEnv->pTimer->GetCurrTime();
	if (aimTime <= 0 || Pos.GetLength() == 0) // Reset
	{ 
		pTurret->m_SC_AimPosition = Vec3(0);
		pTurret->m_SC_WantAimTime = CurrentTime;
	} 
	else
	{
		pTurret->m_SC_AimPosition = Pos;
		pTurret->m_SC_WantAimTime = CurrentTime + aimTime;
	}
	return pH->EndFunction();
}


//------------------------------------------------------------------------
int CScriptBind_SC_GunTurret::StartFire(IFunctionHandler *pH, bool secondary, float fireTime)
{
	CGunTurret* pTurret = GetGunTurret(pH);
	if (!pTurret)
	{
		return pH->EndFunction();
	}

	float CurrentTime = gEnv->pTimer->GetCurrTime();
	if (fireTime <= 0) // Reset
	{ 
		if (secondary)
			pTurret->m_SC_WantFireSec = false;
		else
			pTurret->m_SC_WantFire = false;

		pTurret->m_SC_WantAimTime = CurrentTime;
	} 
	else
	{
		if (secondary)
			pTurret->m_SC_WantFireSec = true;
		else
			pTurret->m_SC_WantFire = true;

		pTurret->m_SC_WantAimTime = CurrentTime + fireTime;
	}
	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_SC_GunTurret::StopFire(IFunctionHandler *pH, bool secondary)
{
	CGunTurret* pTurret = GetGunTurret(pH);
	if (!pTurret)
	{
		return pH->EndFunction();
	}

	if (secondary)
		pTurret->m_SC_WantFireSec = false;
	else
		pTurret->m_SC_WantFire = false;

	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_SC_GunTurret::StopFireAll(IFunctionHandler* pH)
{
	CGunTurret* pTurret = GetGunTurret(pH);
	if (!pTurret)
	{
		return pH->EndFunction();
	}

	pTurret->m_SC_WantFire = false;
	pTurret->m_SC_WantFireSec = false;
	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_SC_GunTurret::SetLookAt(IFunctionHandler *pH, float yaw, float pitch)
{
	CGunTurret* pTurret = GetGunTurret(pH);
	if (!pTurret)
	{
		return pH->EndFunction();
	}

	pTurret->m_SC_YawGoal = yaw;
	pTurret->m_SC_PitchGoal = pitch;
	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_SC_GunTurret::ResetLookAt(IFunctionHandler *pH)
{
	CGunTurret* pTurret = GetGunTurret(pH);
	if (!pTurret)
	{
		return pH->EndFunction();
	}

	pTurret->m_SC_YawGoal = 999.f; // this means.. reset.. oof!
	pTurret->m_SC_PitchGoal = 999.f; // this means.. reset.. oof!
	return pH->EndFunction();
}
