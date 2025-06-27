#include "SafeWriting.h"

#include "CryAction/ActorSystem.h"
#include "CryAction/GameRulesSystem.h"
#include "CryCommon/CryNetwork/INetwork.h"

#include <string>

void CSafeWriting::OnGameRulesLoad(IGameRules* pGR) {
    gEnv->pScriptSystem->ReloadScript("SfW/SafeWritingGameRules.lua");
}

void CSafeWriting::OnGameRulesUnload(IGameRules* pGR) {

}

void CSafeWriting::OnClientConnect(IGameRules *pGR, int channelId, bool isReset) {
    if(INetChannel *pNetChannel=gEnv->pGame->GetIGameFramework()->GetNetChannel(channelId)) {
			int n_ip = *(int*)(((const char*)pNetChannel) + 0x78);
			std::string profileId = std::to_string(pNetChannel->GetProfileId());

			char ip[256];
			std::string hostName { pNetChannel->GetName() };
			if(size_t pos = hostName.find(':'); pos != std::string::npos) {
				hostName = hostName.substr(0, pos);
			}

			snprintf(ip, 255, "%d.%d.%d.%d", (n_ip >> 24) & 255, (n_ip >> 16) & 255, (n_ip >> 8) & 255, (n_ip & 255));

			SmartScriptTable pScript=pGR->GetEntity()->GetScriptTable();
			if(IScriptSystem *pSS = gEnv->pScriptSystem; pSS->BeginCall(pScript, "GatherClientData")) {
				const char *newMessage = NULL;
				pSS->PushFuncParam(pScript);
				pSS->PushFuncParam(channelId);
				pSS->PushFuncParam(pNetChannel->GetName());
				pSS->PushFuncParam(profileId.c_str());
				pSS->PushFuncParam(ip);
				pSS->EndCall();
			}
		}
}

std::optional<std::string> CSafeWriting::OnChatMessage(IGameRules *pGR, EChatMessageType type, EntityId sourceId, EntityId targetId, const std::string& msg) {
    SmartScriptTable pScript=pGR->GetEntity()->GetScriptTable();
    if(IScriptSystem *pSS = gEnv->pScriptSystem; pSS->BeginCall(pScript, "OnChatMessage")) {
        const char *newMessage = NULL;
        pSS->PushFuncParam(pScript);
        pSS->PushFuncParam(type);
        pSS->PushFuncParam(sourceId);
        pSS->PushFuncParam(targetId);
        pSS->PushFuncParam(msg.c_str());
        bool ok = pSS->EndCall(newMessage);
        if(!ok || !newMessage) {
            return {};
        } else {
            return newMessage;
        }
    } else {
        return msg;
    }
}

std::optional<std::string> CSafeWriting::OnPlayerRename(IGameRules *pGR, IActor* pActor, const std::string& name) {
    SmartScriptTable pScript=pGR->GetEntity()->GetScriptTable();
    if(IScriptSystem *pSS = gEnv->pScriptSystem; pSS->BeginCall(pScript, "OnPlayerRename")) {
        const char *newName = NULL;
        pSS->PushFuncParam(pScript);
        pSS->PushFuncParam(pActor->GetEntityId());
        pSS->PushFuncParam(name.c_str());
        bool ok = pSS->EndCall(newName);
        if(!ok || !newName) {
            return {};
        } else {
            return newName;
        }
    } else {
        return name;
    }
}

bool CSafeWriting::IsRMILegitimate(INetChannel* pNC, EntityId sourceId) {
    IGameFramework* pFW = gEnv->pGame->GetIGameFramework();
    IActorSystem* pAS = pFW->GetIActorSystem();
    IActor* client = pAS->GetActorByChannelId(pFW->GetGameChannelId(pNC));
    IActor* claimsToBe = pAS->GetActor(sourceId);
    return client != NULL && client == claimsToBe;
}

bool CSafeWriting::IsHitRMILegitimate(INetChannel* pNC, EntityId sourceId, EntityId weaponId) {
    if (!IsRMILegitimate(pNC, sourceId)) {
        IEntity* pEntity = gEnv->pSystem->GetIEntitySystem()->GetEntity(weaponId);
        if (!pEntity || !strcmp(pEntity->GetClass()->GetName(), "AACannon")) {
            return false;
        } else {
            return true;
        }
    } else {
        return true;
    }
}