#ifndef __SERVERCVARS_H__
#define __SERVERCVARS_H__

#include <cstring>
#include <unordered_map>

#include "CryCommon/CrySystem/IConsole.h"


//struct gServer;
struct IConsole;
struct ICVar;

//std::unordered_map<std::string, ServerCVars::CVarInfo> ServerCVars::m_CVarMap;

struct SC_ServerCVars {

private:
	IConsole* m_pConsole;


public:
	
	// -----------------------
	// A*
	float astar_callsLimitTime;
	int astar_callsLimit;
	int astar_use_cache;
	

	// -----------------------
	// Files
	int filesave_debug;

	int test__var;
	

	// -----------------------
	// Server

	int server_explosion_Limiter;
	int server_debug_vehicleAI;
	int server_debug_vehicleAI_steerVal;
	int server_disable_latencyReset;
	int server_disable_crympcode;
	int server_debug_positions;

	int server_turret_debug;
	int server_turrets_rocketTargetProjectiles;
	int server_turrets_targetOnlyTAC;
	int server_turrets_rocketAutoAim;
	float server_turret_rocketAimRange;
	float server_turret_debug_step;

	int server_hits_processDead;
	float server_hits_processDeadTime;

	int server_projectiles_ignoreOwnerCollision;
	float server_projectiles_ownerCollisionMaxTime;
	float server_projectiles_spawnBehind;

	int server_use_hit_queue;
	int server_use_explosion_queue;
	int server_classic_chat;
	int server_lag_resetmovement;
	int server_ghostbug_fix;
	int server_turrets_target_cloaked;
	int server_autodrop_rpg;
	int server_suit_cloakMeleeEnergyCost;
	int server_suit_cloakShootingEnergyCost;
	int server_suit_speedShootingEnergyCost;
	int server_suit_strengthShootingEnergyCost;
	int server_suit_strengthMeleeEnergyCost;
	int server_suit_speedMeleeEnergyCost;
	int server_c4_limit;
	int server_allow_c4Hits;
	int server_c4_stickToPlayers;
	int server_c4_stickToAllSpecies;
	int server_c4_stickLimitOne;
	int server_fix_spectatorDesync;
	float server_spectatorFix_ResetThreshold;
	float server_anticheat_weaponCheckInterval;
	int server_allow_scan_explosives;
	int server_allow_scan_cloaked;


	// Commands
	ICVar *server_mapTitle;

	// -----------------------
	// CryMP (Synched)
	int			mp_crymp;
	float		mp_circleJump;
	float		mp_wallJump;
	int			mp_flyMode;
	int			mp_pickupObjects;
	int			mp_pickupVehicles;
	int			mp_weaponsOnBack;
	int	        mp_thirdPerson;
	int			mp_animationGrenadeSwitch;
	int			mp_ragdollUnrestricted;
	int			mp_killMessages;
	int			mp_rpgMod;
	int			mp_aaLockOn;
	float		mp_C4StrengthThrowMult;

	// Client (Unsynched, unused)
	int			mp_newSpectator;
	int			mp_usePostProcessAimDir;
	int			mp_messageCenterColor;	
	float		mp_animationWeaponMult;
	float		mp_animationWeaponMultSpeed;
	float		mp_animationModelMult;
	float		mp_animationModelMultSpeed;
	float		mp_menuSpeed;
	int			mp_hitIndicator;
	int			mp_chatHighResolution;
	float		mp_spectatorSlowMult;
	int			mp_buyPageKeepTime;
	int			mp_attachBoughtEquipment;
	float		mp_netAimLerpFactor;
	float		mp_netAimLerpFactorCrymp;
	int         mp_explosiveSilhouettes;

	// -----------------------
	SC_ServerCVars();
	~SC_ServerCVars();

	void InitCVars(IConsole* pConsole);
	void ReleaseCVars();
};

// -----------------------
extern struct SC_ServerCVars* g_pSC_ServerCVars;

#endif // __SERVERCVARS_H__