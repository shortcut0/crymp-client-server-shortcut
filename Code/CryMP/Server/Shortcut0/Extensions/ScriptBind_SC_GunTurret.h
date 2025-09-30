# pragma once

#ifndef __SCRIPTBIND_SERVERGT_H__
#define __SCRIPTBIND_SERVERGT_H__

#include "CryCommon/CryScriptSystem/IScriptSystem.h"

class CGunTurret;
struct IGameFramework;
struct ISystem;


class CScriptBind_SC_GunTurret :
	public CScriptableBase
{
public:
	CScriptBind_SC_GunTurret(ISystem* pSystem, IGameFramework* pGameFramework);
	virtual ~CScriptBind_SC_GunTurret();

	// ==============================
	int SetTarget(IFunctionHandler* pH, ScriptHandle targetId, float aimTime);
	int SetAimTarget(IFunctionHandler* pH, ScriptHandle targetId, float aimTime);
	int SetAimPosition(IFunctionHandler* pH, Vec3 Pos, float aimTime);
	int StartFire(IFunctionHandler* pH, bool secondary, float fireTime);
	int StopFire(IFunctionHandler* pH, bool secondary);
	int StopFireAll(IFunctionHandler* pH);
	int SetLookAt(IFunctionHandler* pH, float yaw, float pitch);
	int ResetLookAt(IFunctionHandler* pH);
	int ResetProperties(IFunctionHandler* pH);
	int OnTargetLocked(IFunctionHandler* pH);
	int ChangeTargetTo(IFunctionHandler* pH);

	// ==============================
	void AttachTo(IEntity* pGunTurret);

protected:

private:
	void RegisterGlobals();
	void RegisterMethods();

	CGunTurret* GetGunTurret(IFunctionHandler* pH);
	CWeapon* GetWeapon(IFunctionHandler* pH);

	ISystem* m_pSystem;
	IScriptSystem* m_pSS;
	IGameFramework* m_pGameFW;
};

#endif //__SCRIPTBIND_SERVERGT_H__
