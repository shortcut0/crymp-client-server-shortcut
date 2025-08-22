# pragma once

#ifndef __SCRIPTBIND_SERVERVEHICLE_H__
#define __SCRIPTBIND_SERVERVEHICLE_H__

#include "CryCommon/CryScriptSystem/IScriptSystem.h"


// Sadly, these are needed..
#include "CryCommon/CryAction/IVehicleSystem.h" // header
#include "CryGame/Vehicles/Movement/VehicleMovementBase.h"
#include "CryGame/Vehicles/Movement/VehicleMovementStdWheeled.h"
#include "CryGame/Vehicles/Movement/VehicleMovementStdBoat.h"
#include "CryGame/Vehicles/Movement/VehicleMovementHelicopter.h"
#include "CryGame/Vehicles/Movement/VehicleMovementVTOL.h"
#include "CryGame/Vehicles/Movement/VehicleMovementHovercraft.h"
#include "CryGame/Vehicles/Movement/VehicleMovementTank.h"

struct IGameFramework;
struct ISystem;

/*
class CVehicleMovementBase;
class CVehicleMovementStdWheeled;
class CVehicleMovementHelicopter;
class CVehicleMovementHovercraft;
class CVehicleMovementStdBoat;
class CVehicleMovementTank;
class CVehicleMovementVTOL;
*/

class CScriptBind_SC_ServerVehicle :
	public CScriptableBase
{
public:
	CScriptBind_SC_ServerVehicle(ISystem* pSystem, IGameFramework* pGameFramework);
	virtual ~CScriptBind_SC_ServerVehicle();

	// ==============================
	void AttachTo(IEntity* pVehicle);

protected:

	void ProcessMovementForController(IFunctionHandler* pH, CMovementRequest &pMC, CMovementRequest &pMC2);

	// ======================================
	int StopMovement(IFunctionHandler* pH);
	int RequestMovement(IFunctionHandler* pH);

	int StopEngine(IFunctionHandler* pH);
	int StartEngine(IFunctionHandler* pH, ScriptHandle driver);
	int DisableEngine(IFunctionHandler* pH, bool disable);
	
	int SetSteering(IFunctionHandler* pH, float steering);
	int SetEnginePower(IFunctionHandler* pH, float power);

	int OnAction(IFunctionHandler* pH, int actionId, int activation, float value);
	int ProcessMovement(IFunctionHandler* pH);
	int ProcessAI(IFunctionHandler* pH);

private:
	void RegisterGlobals();
	void RegisterMethods();

	template <typename T>
	void UpdateNetActions(T* pMovement) { 
		if (pMovement)
		{
			pMovement->m_netActionSync.UpdateObject(pMovement);
		}
	};

	CVehicleMovementStdWheeled* GetMovement_Wheeled(IVehicle* pVehicle) { return static_cast<CVehicleMovementStdWheeled*>(pVehicle->GetMovement()); }
	CVehicleMovementHelicopter* GetMovement_Helicopter(IVehicle* pVehicle) { return static_cast<CVehicleMovementHelicopter*>(pVehicle->GetMovement()); }
	CVehicleMovementHovercraft* GetMovement_Hovercraft(IVehicle* pVehicle) { return static_cast<CVehicleMovementHovercraft*>(pVehicle->GetMovement()); }
	CVehicleMovementStdBoat* GetMovement_Boat(IVehicle* pVehicle) { return static_cast<CVehicleMovementStdBoat*>(pVehicle->GetMovement()); }
	CVehicleMovementTank* GetMovement_Tank(IVehicle* pVehicle) { return static_cast<CVehicleMovementTank*>(pVehicle->GetMovement()); }
	CVehicleMovementVTOL* GetMovement_VTOL(IVehicle* pVehicle) { return static_cast<CVehicleMovementVTOL*>(pVehicle->GetMovement()); }
	IVehicle* GetVehicle(IFunctionHandler* pH);

	ISystem* m_pSystem;
	IScriptSystem* m_pSS;
	IGameFramework* m_pGameFW;
	CVehicleMovementBase* m_pMovementClass;
};

#endif //__SCRIPTBIND_SERVERVEHICLE_H__
