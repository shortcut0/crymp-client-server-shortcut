#pragma once
#include <string>
#include <optional>

struct IActor;
struct IGameRules;
enum EChatMessageType;

struct ISSM {
public:
    virtual void OnClientConnect(IGameRules *pGR, int channelId, bool isReset) = 0;
    virtual std::optional<std::string> OnChatMessage(IGameRules *pGR, EChatMessageType type, EntityId sourceId, EntityId targetId, const std::string& msg) = 0;
    virtual std::optional<std::string> OnPlayerRename(IGameRules *pGR, IActor* pActor, const std::string& name) = 0;
    virtual bool IsRMILegitimate(INetChannel* pNC, EntityId sourceId) = 0;
    virtual bool IsHitRMILegitimate(INetChannel* pNC, EntityId sourceId, EntityId weaponId) = 0;
};