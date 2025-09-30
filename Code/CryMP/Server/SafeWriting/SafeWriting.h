#pragma once
#include "CryMP/Server/SSM.h"
#include <memory>

struct IGameFramework;
struct ISystem;
struct SafeWritingAPI;
class FunctionRegisterer;

class CSafeWriting : public ISSM {
    bool m_initialized = false;
    std::shared_ptr<SafeWritingAPI> m_pAPI;
    std::unique_ptr<FunctionRegisterer> m_pFR;
public:
    CSafeWriting(IGameFramework *pFW, ISystem *pSystem);
    ~CSafeWriting();
    void OnGameRulesLoad(IGameRules* pGR) override;
    void OnGameRulesUnload(IGameRules* pGR) override;
    void Update(float dt) override;
    void OnClientConnect(IGameRules *pGR, int channelId, bool isReset) override;
    std::optional<std::string> OnChatMessage(IGameRules* pGR, EChatMessageType type, EntityId sourceId, EntityId targetId, const std::string& msg) override;
    std::optional<std::string> OnPlayerRename(IGameRules *pGR, IActor* pActor, const std::string& name) override;
    bool IsRMILegitimate(INetChannel* pNC, EntityId sourceId) override;
    bool IsHitRMILegitimate(INetChannel* pNC, EntityId sourceId, EntityId weaponId) override;
    bool CanReceiveChatMessage(EChatMessageType type, EntityId sourceId, EntityId targetId) override;

    // Shortcut0
    // Added empty functions here
    bool CheckRadioRequest(INetChannel*pNetChnl,EntityId senderId,uint8 msg) { return true; };
    bool CheckMeleeRequest(IEntity*pClient,IEntity*pFists) { return true; };
    bool ChatLog(EChatMessageType type, EntityId senderId, EntityId targetId, const char* msg) { return false; };
    void OnClientEnteredGame(IGameRules* pGR, int channelId, bool isReset) { return ; };
    void OnClientDisconnect(IGameRules* pGR, int channelId, int cause, const char* desc) { return ; };
    bool CanStartNextLevel() { return true;};
    bool LoadAutoexec() { return true; };
     bool LogScriptErrors() { return true; };
     void OnScriptError(const std::string& message) { return ; };
     bool OverwriteScriptPath(std::string &output, const std::string& input) { return false; };
     void OnLoadingScript(const std::string& filePath) { return ; };
     void OnScriptLoaded(const std::string& filePath, bool success) { return ; };
     bool ProcessServerExplosion(ExplosionInfo& OverwriteInfo) { return true; };
     bool OnChatMessageEx(SmartScriptTable& Modifications, EChatMessageType msgType, EntityId senderId, EntityId targetId, const char* msg, bool IsFakeMessage) { return false; };//false because we DO NOT provide a modified table
     void OnVehicleInit(IEntity* pVehicle) { return ; };
     void OnGunTurretInit(IEntity* pGunTurret) { return ; };
     void OnWeaponInit(CWeapon* pWeapon) { return; };
     void OnWallJump(EntityId ownerId, EntityId weaponId) { return; };
     bool CanAttachAccessory(IScriptTable* pOwner, IScriptTable* pItem, const char* accessoryName) { return true; };
     void OnEnterWeaponModify(IScriptTable* pOwner, IScriptTable* pItem) { return ; };
     void OnLeaveWeaponModify(IScriptTable* pOwner, IScriptTable* pItem) { return ; };
     void OnWeaponStartFire(IScriptTable* pOwner, IScriptTable* pWeapon, EntityId ammoId, const char* ammoClass, Vec3 Pos, Vec3 Hit, Vec3 Dir) { return ; };
     void OnPlayerExplosiveDestroyed(EntityId playerId, EntityId explosiveId, int typeId, int numRemaining, bool HasExploded) { return ; };
     void OnPlayerExplosivePlaced(EntityId playerId, EntityId explosiveId, int typeId, int numRemaining, int numLimit) { return ; };
     void OnPlayerRequestHitAssistance(EntityId playerId, bool enable) { return ; };
     bool CheckRocketCollision(CRocket* pRocket, EntityId ownerId, IEntity* pCollidee) { return true; };
     void OnRadarScanComplete(EntityId onwerId, EntityId weaponId, float radius) { return ; };
     bool CheckProjectileHit(EntityId shooterId, EntityId projectileId, bool destroyed, float damage, EntityId weaponId, Vec3 Pos, Vec3 Normal) { return true; };
     bool OnActorDropItem(EntityId ownerId, EntityId itemId) { return true; };
     bool CanActorPickUpItem(EntityId ownerId, EntityId itemId, bool isObjectGrab) { return true; };
     bool CanActorUseItem(EntityId ownerId, EntityId itemId) { return true; };
};