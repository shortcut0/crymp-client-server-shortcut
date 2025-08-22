#include "CryCommon/CrySystem/ISystem.h"
#include "CryCommon/CryEntitySystem/IEntity.h"

#include "ScriptBind_SC_Vehicle.h"

//------------------------------------------------------------------------
CScriptBind_SC_ServerVehicle::CScriptBind_SC_ServerVehicle(ISystem* pSystem, IGameFramework* pGameFramework)
	: m_pSystem(pSystem),
	m_pSS(pSystem->GetIScriptSystem()),
	m_pGameFW(pGameFramework)
{
	Init(m_pSS, m_pSystem, 1);
	SetGlobalName("ServerVehicle");

	RegisterMethods();
	RegisterGlobals();
}

//------------------------------------------------------------------------
CScriptBind_SC_ServerVehicle::~CScriptBind_SC_ServerVehicle()
{
}

//------------------------------------------------------------------------
void CScriptBind_SC_ServerVehicle::RegisterGlobals()
{
}

//------------------------------------------------------------------------
void CScriptBind_SC_ServerVehicle::RegisterMethods()
{
#undef SCRIPT_REG_CLASSNAME
#define SCRIPT_REG_CLASSNAME &CScriptBind_SC_ServerVehicle::

	SCRIPT_REG_FUNC(StopMovement);
	SCRIPT_REG_FUNC(RequestMovement);

	SCRIPT_REG_TEMPLFUNC(DisableEngine, "disable");
	SCRIPT_REG_TEMPLFUNC(StartEngine, "driver");
	SCRIPT_REG_FUNC(StopEngine);
	SCRIPT_REG_FUNC(ProcessMovement);
	SCRIPT_REG_FUNC(ProcessAI);

	SCRIPT_REG_TEMPLFUNC(SetSteering, "steer"); // Boats and Wheeled
	SCRIPT_REG_TEMPLFUNC(SetEnginePower, "power");
	SCRIPT_REG_TEMPLFUNC(OnAction, "action, activation, value");

#undef SCRIPT_REG_CLASSNAME
}

//------------------------------------------------------------------------
void CScriptBind_SC_ServerVehicle::AttachTo(IEntity* pVehicle)
{
	// Invalid Checks + Impossible scenario!
	/*
	if (this == nullptr || !m_pSS)
	{
		return CryLogWarning("CScriptBind_SC_ServerVehicle::AttachTo Called, but the System has not yet been Initialized!");
	}
	*/

	IScriptTable* pScriptTable = pVehicle->GetScriptTable();
	if (pScriptTable)
	{
		SmartScriptTable thisTable(m_pSS);
		thisTable->SetValue("__this", ScriptHandle(pVehicle->GetId()));
		thisTable->Delegate(GetMethodsTable());

		pScriptTable->SetValue("SvVehicle", thisTable);
	}

	//m_pMovementClass = pMovement;
}

//------------------------------------------------------------------------
IVehicle* CScriptBind_SC_ServerVehicle::GetVehicle(IFunctionHandler* pH)
{
	void* pThis = pH->GetThis();
	if (pThis)
	{
		IVehicle* pVehicle = gEnv->pGame->GetIGameFramework()->GetIVehicleSystem()->GetVehicle((EntityId)(UINT_PTR)pThis);
		if (pVehicle)

		{
			return pVehicle;
		}
	}

	return 0;
}

//------------------------------------------------------------------------
void CScriptBind_SC_ServerVehicle::ProcessMovementForController(IFunctionHandler* pH, CMovementRequest& pMC, CMovementRequest& pMC2)
{

	// declare
	Vec3 vForcedNavigation = Vec3(ZERO);
	Vec3 vMoveTarget = Vec3(ZERO);
	Vec3 vFireTarget = Vec3(ZERO);
	Vec3 vAimTarget = Vec3(ZERO);
	Vec3 vLookAt = Vec3(ZERO);
	float fDesiredSpeed = -1.f;
	float fPesudoSpeed = -1.f;

	// resolve
	SmartScriptTable pRequest;
	if (pH->GetParamCount() >= 1)
	{
		if (pH->GetParam(1, pRequest) && pRequest.GetPtr())
		{
			pRequest->GetValue("MoveTarget", vMoveTarget);
			pRequest->GetValue("ForcedNavigation", vForcedNavigation);
			pRequest->GetValue("FireTarget", vFireTarget);
			pRequest->GetValue("AimTarget", vAimTarget);
			pRequest->GetValue("LookAt", vLookAt);

			pRequest->GetValue("DesiredSpeed", fDesiredSpeed);
			pRequest->GetValue("PesudoSpeed", fPesudoSpeed);


			// ----
			float Steering = -1.f;
			if (pRequest->GetValue("SteeringValue", Steering))// && Steering != -1.f)
				SetSteering(pH, Steering);

			float EnginePower = -1.f;
			if (pRequest->GetValue("EnginePower", EnginePower))// && EnginePower != -1.f)
				SetEnginePower(pH, EnginePower);

			int EngineState = -1.f;
			if (pRequest->GetValue("EngineState", EngineState))// && EngineState != -1.f)
			{
				ScriptHandle DriverHandle;
				pRequest->GetValue("DriverId", DriverHandle);
				if (EngineState == 1)
				{
					DisableEngine(pH, false);
					StartEngine(pH, DriverHandle);
				}
				else
					StopEngine(pH);
			}
		}
	}

	// update
	if (fDesiredSpeed != -1.f)
		pMC.SetDesiredSpeed(fDesiredSpeed);
	else
		pMC.ClearDesiredSpeed();

	if (fPesudoSpeed != -1.f)
		pMC.SetPseudoSpeed(fPesudoSpeed);
	else
		pMC.ClearPseudoSpeed();

	if (vMoveTarget.GetLength() > 0)
		pMC.SetMoveTarget(vMoveTarget);
	else
		pMC.ClearMoveTarget();

	if (vAimTarget.GetLength() > 0)
		pMC.SetAimTarget(vAimTarget);
	else
		pMC.ClearAimTarget();

	if (vLookAt.GetLength() > 0)
		pMC.SetLookTarget(vAimTarget);
	else
		pMC.ClearLookTarget();

	if (vForcedNavigation.GetLength() > 0)
		pMC.SetForcedNavigation(vForcedNavigation);
	else
		pMC.ClearForcedNavigation();

	if (vFireTarget.GetLength() > 0)
		pMC.SetFireTarget(vFireTarget);
	else
		pMC.ClearFireTarget();
}

//------------------------------------------------------------------------
int CScriptBind_SC_ServerVehicle::DisableEngine(IFunctionHandler* pH, bool disable)
{
	IVehicle* pVehicle = GetVehicle(pH);
	if (!pVehicle)
	{
		return pH->EndFunction();
	}

	pVehicle->GetMovement()->DisableEngine(disable);
	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_SC_ServerVehicle::StartEngine(IFunctionHandler* pH, ScriptHandle driver)
{
	IVehicle* pVehicle = GetVehicle(pH);
	if (!pVehicle)
	{
		return pH->EndFunction();
	}

	pVehicle->GetMovement()->StartEngine(EntityId(driver.n));
	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_SC_ServerVehicle::StopEngine(IFunctionHandler* pH)
{
	IVehicle* pVehicle = GetVehicle(pH);
	if (!pVehicle)
	{
		return pH->EndFunction();
	}

	pVehicle->GetMovement()->StopEngine();
	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_SC_ServerVehicle::SetSteering(IFunctionHandler* pH, float steering)
{
	IVehicle* pVehicle = GetVehicle(pH);
	if (!pVehicle)
	{
		return pH->EndFunction();
	}

	if (CVehicleMovementStdWheeled* pStdWheeled = GetMovement_Wheeled(pVehicle))
	{
		CryLogAlways("Steer set to %f", steering);
		//pStdWheeled->m_action.steer = steering;
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_SC_ServerVehicle::OnAction(IFunctionHandler* pH, int actionId, int activation, float value)
{
	IVehicle* pVehicle = GetVehicle(pH);
	if (!pVehicle)
	{
		return pH->EndFunction();
	}

	pVehicle->GetMovement()->OnAction(TVehicleActionId(actionId), (activation > 0 ? eAAM_OnPress : eAAM_OnRelease), value);

	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_SC_ServerVehicle::SetEnginePower(IFunctionHandler* pH, float power)
{
	IVehicle* pVehicle = GetVehicle(pH);
	if (!pVehicle)
	{
		return pH->EndFunction();
	}

	if (CVehicleMovementStdWheeled* pStdWheeled = GetMovement_Wheeled(pVehicle))
	{
		//pStdWheeled->m_action.pedal = (power > 0 ? 1.f : 0.f);
		//pStdWheeled->m_carParams.enginePower = power;
		//pStdWheeled->m_movementAction.power = power;
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_SC_ServerVehicle::StopMovement(IFunctionHandler* pH)
{
	IVehicle* pVehicle = GetVehicle(pH);
	if (!pVehicle)
	{
		return pH->EndFunction();
	}

	if (IVehicleMovement* pVM = pVehicle->GetMovement())
	{
		CMovementRequest pMR;
		pMR.ClearMoveTarget();
		pVM->RequestMovement(pMR);
		//pVM->EnableMovementProcessing(false); // Is this safe?
	}

	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_SC_ServerVehicle::ProcessAI(IFunctionHandler* pH)
{
	IVehicle* pVehicle = GetVehicle(pH);
	if (!pVehicle)
	{
		return pH->EndFunction();
	}

	if (IVehicleMovement* pVM = pVehicle->GetMovement())
	{

		// WHEELED
		if (CVehicleMovementStdWheeled* pMovement = GetMovement_Wheeled(pVehicle))
		{
			pMovement->ProcessAI(gEnv->pTimer->GetFrameTime());
		}

		// HELI
		else if (CVehicleMovementHelicopter* pMovement = GetMovement_Helicopter(pVehicle))
		{
			pMovement->ProcessAI(gEnv->pTimer->GetFrameTime());
		}

	}
	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_SC_ServerVehicle::ProcessMovement(IFunctionHandler* pH)
{
	IVehicle* pVehicle = GetVehicle(pH);
	if (!pVehicle)
	{
		return pH->EndFunction();
	}

	if (IVehicleMovement* pVM = pVehicle->GetMovement())
	{
		pVM->ProcessMovement(gEnv->pTimer->GetFrameTime());
		if (CVehicleMovementStdWheeled* pMovement = GetMovement_Wheeled(pVehicle))
		{
			pMovement->ProcessMovement(gEnv->pTimer->GetFrameTime());
		}

		// HELI
		else if (CVehicleMovementHelicopter* pMovement = GetMovement_Helicopter(pVehicle))
		{
			pMovement->ProcessMovement(gEnv->pTimer->GetFrameTime());
		}

	}
	return pH->EndFunction();
}

//------------------------------------------------------------------------
int CScriptBind_SC_ServerVehicle::RequestMovement(IFunctionHandler* pH)
{
	IVehicle* pVehicle = GetVehicle(pH);
	if (!pVehicle)
	{
		return pH->EndFunction();
	}

	if (IVehicleMovement* pVM = pVehicle->GetMovement())
	{
		/*
		CMovementRequest pMR;
		pMR.SetMoveTarget(Vec3(100, 100, 1000));
		pMR.SetDesiredSpeed(10);
		pMC->RequestMovement(pMR);
		*/

		float fDeltaTime = gEnv->pTimer->GetFrameTime();
		pVM->EnableMovementProcessing(true);

		if (CVehicleMovementStdWheeled* pStdWheeled = GetMovement_Wheeled(pVehicle))
		{
			//CMovementRequest pRequest;
			//CMovementRequest pAIRequest = pStdWheeled->m_aiRequest;
			//ProcessMovementForController(pH, pRequest, pRequest);
			//ProcessMovementForController(pH, pAIRequest, pAIRequest);
			//pStdWheeled->ProcessMovement(fDeltaTime);
			//pStdWheeled->ProcessAI(fDeltaTime);
			//pStdWheeled->RequestMovement(pRequest);

			//UpdateNetActions(pStdWheeled);
			//UpdateNetActions(pStdWheeled); // done already?
		}

		// HELI
		else if (CVehicleMovementHelicopter* pMovement = GetMovement_Helicopter(pVehicle))
		{
			//CMovementRequest pRequest;
			//ProcessMovementForController(pH, pRequest, pRequest);
			//pStdWheeled->RequestMovement(pRequest);
			//pStdWheeled->ProcessAI(fDeltaTime);
			//pStdWheeled->ProcessMovement(fDeltaTime);
		}
		// else if () ...

	}
	return pH->EndFunction();
}

/*
eVMT_Sea = 0,
	eVMT_Air,
	eVMT_Land,
	eVMT_Amphibious,
	eVMT_Other

	pVehicle->GetMovement()->StartEngine(EntityId(0));
*/