#pragma once
#include "CryMP/Server/SSM.h"

class CSafeWriting : public ISSM {
public:
    void OnClientConnect(IGameRules *pGR, int channelId, bool isReset) override;
    std::optional<std::string> OnChatMessage(IGameRules* pGR, EChatMessageType type, EntityId sourceId, EntityId targetId, const std::string& msg) override;
    std::optional<std::string> OnPlayerRename(IGameRules *pGR, IActor* pActor, const std::string& name) override;
    bool IsRMILegitimate(INetChannel* pNC, EntityId sourceId) override;
    bool IsHitRMILegitimate(INetChannel* pNC, EntityId sourceId, EntityId weaponId) override;
};