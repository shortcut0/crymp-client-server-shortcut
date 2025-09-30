#include "CryGame/Actors/Actor.h"
#include "..\SC_ServerEvents.h"
#include "CryMP/Server/SSM.h"

#include <cmath>  // For sqrt and pow functions

class SC_ServerAnticheat
{

private:

	SC_ServerEvents* m_pEvents;
	IGameFramework* m_pGameFramework;
	bool m_status = true;

	CActor* GetActor(EntityId actorId)
	{
		return static_cast<CActor*>(m_pGameFramework->GetIActorSystem()->GetActor(actorId));
	}

	CActor* GetActorByChannel(uint16 actorId)
	{
		return static_cast<CActor*>(m_pGameFramework->GetIActorSystem()->GetActorByChannelId(actorId));
	}

public:

	// ------------------------------
	SC_ServerAnticheat() {};
	~SC_ServerAnticheat() {};

	void Init(IGameFramework* pGameFramework, SC_ServerEvents* pEvents) {
		m_pGameFramework = pGameFramework; 
		m_pEvents = pEvents; 
	};

	// -----------------
	void ToggleStatus(bool mode) { m_status = mode; };

	// -----------------
	template<class... Params>
	bool Distance(INetChannel* pNetChannel, Vec3 from, Vec3 to, float threshold, const char* sCheatName, const char* sFunctionPtr, const Params &... params) {
		float distance = std::sqrt(std::pow(to.x - from.x, 2) +
			std::pow(to.y - from.y, 2) +
			std::pow(to.z - from.z, 2));
		
		if (!pNetChannel || distance <= threshold)
			return true;

		// try to handle cheater.. if they exist..
		if (uint16 pChannelId = m_pGameFramework->GetGameChannelId(pNetChannel)) {
			if (CActor* pNetActor = GetActorByChannel(pChannelId))
			{
				EntityId pNetId = pNetActor->GetEntityId();
				m_pEvents->Call(SERVER_SCRIPT_EVENT_OnCheat, pChannelId, sCheatName, sFunctionPtr, ScriptHandle(pNetId), ScriptHandle(pNetId), false, params...);
			}
		}

		return false;
	}

	// -----------------
	template<class... Params>
	bool CheckDistance(INetChannel* pNetChannel, const Params &... params) { return Distance(pNetChannel, params...); } // ?!

	// -----------------

	//template<class... Params>
	bool CheckOwnerRequest(RMIRequest &pRequest) {
		if (!m_status)
		{
			return true; // ok
		}

		INetChannel* pNetChannel = pRequest.NetChannel;
		uint16 pChannelId = m_pGameFramework->GetGameChannelId(pNetChannel);
		if (!pChannelId) 
		{
			return false; // channel not found, drop request
		}

		CActor* pNetActor = GetActorByChannel(pChannelId);
		if (!pNetActor) 
		{
			return false; // no actor associated with this channel, drop request
		}

		EntityId pOwnerId = pRequest.PretendId;
		CActor* pOwnerActor = GetActor(pOwnerId);
		if (!pOwnerActor) 
		{
			return false; // owner actor not found, drop request
		}

		EntityId pNetId = pNetActor->GetEntityId();
		EntityId pActorId = pOwnerActor->GetEntityId();

		// Special case for laggy players, might send RMI upon death, causing a false positive
		if (pNetId != pActorId && (pActorId == NULL || pNetId == NULL))
		{
			return false;
		}

		// We only call this if the ID has been spoofed!
		const char* sCheatName = pRequest.CheatName;
		const char* sFunctionPtr = pRequest.Function;
		if (pActorId != pNetId && pOwnerActor->GetChannelId() != pChannelId) {
			m_pEvents->Call(SERVER_SCRIPT_EVENT_OnCheat, pChannelId, sCheatName, sFunctionPtr, ScriptHandle(pNetId), ScriptHandle(pActorId));
			return false; // owner does not match channel owner, probable cheat!!
		}

		return true; // request ok!
	}

	bool CheckLongpoke(CActor* pActor, int seq) {
		if (!m_status) { 
			return false;  // no longpoke
		} 
		return seq == 1; // SEQ == 1 means longpoke ??
	};
};