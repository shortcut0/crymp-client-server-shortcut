#pragma once
#pragma warning(disable : 4471) // 'EChatMessageType': a forward declaration of an unscoped enumeration must have an underlying type | Annoying!

#include <string>
#include <optional>

#include "CryCommon/CryEntitySystem/EntityId.h"
#include "CryCommon/CryMath/Cry_Math.h"


struct IActor;
struct IGameRules;
struct INetChannel; 
enum EChatMessageType; // ok


// Shortcut0
struct IEntity;
struct ExplosionInfo;
struct IScriptTable;
class SmartScriptTable;
class CRocket;
class CWeapon;
//struct Vec3;

struct RMIRequest { // SC FIXME
    const char* Function;
    const char* CheatName;
    INetChannel* NetChannel;
    EntityId PretendId;
    EntityId VictimId;
};

struct ISSM {
public:
    ~ISSM() = default;
    virtual void OnGameRulesLoad(IGameRules * pGR) = 0;
    virtual void OnGameRulesUnload(IGameRules* pGR) = 0;
    virtual void Update(float dt) = 0;
    virtual void OnClientConnect(IGameRules *pGR, int channelId, bool isReset) = 0;
    virtual bool IsHitRMILegitimate(INetChannel* pNC, EntityId sourceId, EntityId weaponId) = 0;
    virtual bool CanReceiveChatMessage(EChatMessageType type, EntityId sourceId, EntityId targetId) = 0;

    virtual bool IsRMILegitimate(INetChannel* pNC, EntityId sourceId) = 0;

    virtual std::optional<std::string> OnChatMessage(IGameRules* pGR, EChatMessageType type, EntityId sourceId, EntityId targetId, const std::string& msg) = 0;
    virtual std::optional<std::string> OnPlayerRename(IGameRules* pGR, IActor* pActor, const std::string& name) = 0;

    // Shortcut0
    //virtual bool CheckRMIRequest(RMIRequest& pRequest) = 0;
    virtual bool CheckRadioRequest(INetChannel *pChn, EntityId pClient, uint8 msg) = 0;
    virtual bool CheckMeleeRequest(IEntity* pClient, IEntity* pFists) = 0;
    virtual bool ChatLog(EChatMessageType type, EntityId senderId, EntityId targetId, const char* msg) = 0;
    virtual void OnClientEnteredGame(IGameRules* pGR, int channelId, bool isReset) = 0;
    virtual void OnClientDisconnect(IGameRules* pGR, int channelId, int cause, const char* desc) = 0;
    virtual bool CanStartNextLevel() = 0;
    virtual bool LoadAutoexec() = 0;                                                    // Called prior to loading autoexec, return FALSE to not load that thing
    virtual bool LogScriptErrors() = 0;                                                 // Return TRUE to log script errors
    virtual void OnScriptError(const std::string& message) = 0;                         // Called when a script error occurs
    virtual bool OverwriteScriptPath(std::string &output, const std::string& input) = 0; // Asks if a Script file path should be overwritten prior to loading
    virtual void OnLoadingScript(const std::string& filePath) = 0;                      // Called BEFORE a script is being loaded
    virtual void OnScriptLoaded(const std::string& filePath, bool success) = 0;         // Called AFTER a script loaded (success will be false if an error occured)
    virtual bool ProcessServerExplosion(ExplosionInfo& OverwriteInfo) = 0;              // Called while an explosion is being processed. you can modify members of the info here to dynamically change effects and parameters. Return FALSE to abort the explosion processing!
    virtual bool OnChatMessageEx(SmartScriptTable& Modifications, EChatMessageType msgType, EntityId senderId, EntityId targetId, const char* msg, bool IsFakeMessage) = 0;
    virtual void OnVehicleInit(IEntity* pVehicle) = 0;
    virtual void OnGunTurretInit(IEntity* pGunTurret) = 0;
    virtual void OnWeaponInit(CWeapon* pWeapon) = 0;
    virtual bool CanAttachAccessory(IScriptTable* pOwner, IScriptTable* pItem, const char* accessoryName) = 0;
    virtual void OnEnterWeaponModify(IScriptTable* pOwner, IScriptTable* pItem) = 0;
    virtual void OnLeaveWeaponModify(IScriptTable* pOwner, IScriptTable* pItem) = 0;
    virtual void OnWeaponStartFire(IScriptTable* pOwner, IScriptTable* pWeapon, EntityId ammoId, const char* ammoClass, Vec3 Pos, Vec3 Hit, Vec3 Dir) = 0;
    virtual void OnPlayerExplosiveDestroyed(EntityId playerId, EntityId explosiveId, int typeId, int numRemaining, bool HasExploded) = 0;
    virtual void OnPlayerExplosivePlaced(EntityId playerId, EntityId explosiveId, int typeId, int numRemaining, int numLimit) = 0;
    virtual void OnPlayerRequestHitAssistance(EntityId playerId, bool enable) = 0;
    virtual bool CheckRocketCollision(CRocket* pRocket, EntityId ownerId, IEntity* pCollidee) = 0;
    virtual void OnRadarScanComplete(EntityId onwerId, EntityId weaponId, float radius) = 0;
    virtual bool CheckProjectileHit(EntityId shooterId, EntityId projectileId, bool destroyed, float damage, EntityId weaponId, Vec3 Pos, Vec3 Normal) = 0;
    virtual bool OnActorDropItem(EntityId ownerId, EntityId itemId) = 0;
    virtual bool CanActorPickUpItem(EntityId ownerId, EntityId itemId, bool isObjectGrab) = 0;
    virtual bool CanActorUseItem(EntityId ownerId, EntityId itemId) = 0;
    virtual void OnWallJump(EntityId ownerId, EntityId weaponId) = 0;
};
