#pragma once
#include "CryMP/Server/SSM.h"

struct SafeWritingAPI;
class FunctionRegisterer;

class CSafeWriting : public ISSM {
    bool m_initialized = false;
    SafeWritingAPI* m_pAPI;
    FunctionRegisterer* m_pFR;
public:
    CSafeWriting();
    ~CSafeWriting();
    void Update(float dt) override;
    void OnClientConnect(IGameRules *pGR, int channelId, bool isReset) override;
    std::optional<std::string> OnChatMessage(IGameRules* pGR, EChatMessageType type, EntityId sourceId, EntityId targetId, const std::string& msg) override;
    std::optional<std::string> OnPlayerRename(IGameRules *pGR, IActor* pActor, const std::string& name) override;
    bool IsRMILegitimate(INetChannel* pNC, EntityId sourceId) override;
    bool IsHitRMILegitimate(INetChannel* pNC, EntityId sourceId, EntityId weaponId) override;
    bool CanReceiveChatMessage(EChatMessageType type, EntityId sourceId, EntityId targetId) override;
};