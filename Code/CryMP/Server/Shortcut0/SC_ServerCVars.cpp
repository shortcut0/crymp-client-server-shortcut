/*************************************************************************
  Crytek Source File.
  Copyright (C), Crytek Studios, 2001-2004.
 -------------------------------------------------------------------------
  $Id$
  $DateTime$

 -------------------------------------------------------------------------
  History:
  - 11:8:2004   10:50 : Created by Márcio Martins

*************************************************************************/

#include "SC_Server.h"
#include "SC_ServerCVars.h"
#include "Utils\SC_CVarCallbacks.h"

//std::unordered_map<std::string, SC_ServerCVars::CVarInfo> SC_ServerCVars::m_CVarMap;

SC_ServerCVars::SC_ServerCVars()
{
	std::memset(this, 0, sizeof(SC_ServerCVars));
}

SC_ServerCVars::~SC_ServerCVars()
{
	this->ReleaseCVars();
}

void SC_ServerCVars::InitCVars(IConsole* pConsole)
{

	m_pConsole = pConsole;

	// ---------------------------------------------------------
	// Callbacks

	if (ICVar* pMaxPlayers = pConsole->GetCVar("sv_maxPlayers"))
	{
		pMaxPlayers->SetOnChangeCallback(SC_ServerCCallbacks::Sv_MaxPlayers);
	}

	if (ICVar* pServerName = pConsole->GetCVar("sv_serverName"))
	{
		pServerName->SetOnChangeCallback(SC_ServerCCallbacks::Sv_ServerName);
	}

	//pConsole->RemoveCommand("map");
	//pConsole->AddCommand("map", ServerCCallbacks::NewMapCommand);

	// Flags
	const int OPTIONAL_SYNC = 0;

	// ---------------
	// CVars
	pConsole->Register("Server_LogVerbosity", &LogVerbosity, 0, 0, "Logging Verbosity");
	

	// TODO all of THESE

	pConsole->Register("filesave_debug", &filesave_debug, 0, 0, "Enables debugging and testing the ASYNC server file saver");
	pConsole->Register("astar_use_cache", &astar_use_cache, 0, 0, "Enables using cached navmesh for generating new paths");
	pConsole->Register("astar_callsLimit", &astar_callsLimit, 10, 0, "Sets threshold for astar calls per second");
	pConsole->Register("astar_callsLimitTime", &astar_callsLimitTime, 1, 0, "");

	pConsole->Register("server_hits_useQueue", &server_use_hit_queue, 1, 0, "enables the hit queue");
	pConsole->Register("server_hits_processDead", &server_hits_processDead, 1, 0, "enables processing hits even afte the owner died");
	pConsole->Register("server_hits_processDeadTime", &server_hits_processDeadTime, 1, 0, "sets the timeout for processing death hits");

	pConsole->Register("server_projectiles_ignoreOwnerCollision", &server_projectiles_ignoreOwnerCollision, 1, 0, "Enables server ignoring collisions with projectile owner (rockets)");
	pConsole->Register("server_projectiles_ownerCollisionMaxTime", &server_projectiles_ownerCollisionMaxTime, 2, 0, "Time after which collisions with owner are registered again");
	pConsole->Register("server_projectiles_spawnBehind", &server_projectiles_spawnBehind, 5, 0, "[debug]spawns projectiles this amout behind their initial pos");

	pConsole->Register("server_explosion_Limiter", &server_explosion_Limiter, 25, 0, "Maximum amount of explosions allowed to spawn per second");
	pConsole->Register("server_debug_vehicleAI", &server_debug_vehicleAI, 0, 0, "Enable/Disable the ghost bug fix");
	pConsole->Register("server_debug_vehicleAI_steerVal", &server_debug_vehicleAI_steerVal, 0, 0, "Enable/Disable the ghost bug fix");
	pConsole->Register("server_disable_latencyReset", &server_disable_latencyReset, 1, 0, "Enable/Disable the ghost bug fix");
	pConsole->Register("server_disable_crympcode", &server_disable_crympcode, 1, 0, "Enable/Disable the ghost bug fix");
	pConsole->Register("server_debug_positions", &server_debug_positions, 0, 0, "Enable/Disable the ghost bug fix");

	pConsole->Register("server_turret_rocketAimRange", &server_turret_rocketAimRange, 150, 0, "Enable/Disable the ghost bug fix");
	pConsole->Register("server_turrets_rocketTargetProjectiles", &server_turrets_rocketTargetProjectiles, 1, 0, "Enable/Disable the ghost bug fix");
	pConsole->Register("server_turrets_rocketAutoAim", &server_turrets_rocketAutoAim, 1, 0, "Enable/Disable the ghost bug fix");
	pConsole->Register("server_turrets_targetOnlyTAC", &server_turrets_targetOnlyTAC, 0, 0, "Enable/Disable the ghost bug fix");
	pConsole->Register("server_turret_debug", &server_turret_debug, 0, 0, "Enable/Disable the ghost bug fix");
	pConsole->Register("server_turret_debug_step", &server_turret_debug_step, 0.05, 0, "Enable/Disable the ghost bug fix");

	pConsole->Register("server_ghostbug_fix", &server_ghostbug_fix, 0, 0, "Enable/Disable the ghost bug fix");
	pConsole->Register("server_use_hit_queue", &server_use_hit_queue, 0, 0, "Enable/Disable the hit queue");
	pConsole->Register("server_use_explosion_queue", &server_use_explosion_queue, 1, 0, "Enable/Disable the explosion queue");
	pConsole->Register("server_classic_chat", &server_classic_chat, 0, 0, "Enable/Disable the default chat system");
	pConsole->Register("server_turrets_target_cloaked", &server_turrets_target_cloaked, 0, 0, "Enable/Disable turrets targetting cloaked players");
	pConsole->Register("server_autodrop_rpg", &server_autodrop_rpg, 0, 0, "Enable/Disable automatically dropping RPGs");
	pConsole->Register("server_c4_limit", &server_c4_limit, 30, 0, "Controls how much C4 a player can own");
	pConsole->Register("server_allow_c4Hits", &server_allow_c4Hits, 1, 0, "Enable/Disable C4 hit registry");
	pConsole->Register("server_allow_scan_explosives", &server_allow_scan_explosives, 1, 0, "Enable/Disable C4 hit registry");
	pConsole->Register("server_allow_scan_cloaked", &server_allow_scan_cloaked, 0, 0, "Enable/Disable C4 hit registry");
	pConsole->Register("server_c4_stickToPlayers", &server_c4_stickToPlayers, 1, 0, "Enable/Disable c4 sticking to players");
	pConsole->Register("server_c4_stickToAllSpecies", &server_c4_stickToAllSpecies, 1, 0, "Enable/Disable c4 sticking to all actor species (grunts,aliens,etc)");
	pConsole->Register("server_c4_stickLimitOne", &server_c4_stickLimitOne, 0, 0, "Enable/Disable limits the amont of c4 that can stick on an actor to 1");
	pConsole->Register("server_fix_spectatorDesync", &server_fix_spectatorDesync, 0, 0, "Enable/Disable attempts to fix the spectator position desync");
	pConsole->Register("server_spectatorFix_ResetThreshold", &server_spectatorFix_ResetThreshold, 1, 0, "Enable/Disable attempts to fix the spectator position desync");

	pConsole->Register("server_anticheat_weaponCheckInterval", &server_anticheat_weaponCheckInterval, (float)(75 / 100), 0, "Enable/Disable attempts to fix the spectator position desync");

	pConsole->Register("server_suit_cloakMeleeEnergyCost", &server_suit_cloakMeleeEnergyCost, 20, 0, "Energy cost when performing a melee attack while using cloak mode");
	pConsole->Register("server_suit_cloakShootingEnergyCost", &server_suit_cloakShootingEnergyCost, 20, 0, "Energy cost when firing a weapon while using cloak mode");

	pConsole->Register("server_suit_speedMeleeEnergyCost", &server_suit_speedMeleeEnergyCost, 25, 0, "Energy cost when performing a melee attack while using cloak mode");
	pConsole->Register("server_suit_speedShootingEnergyCost", &server_suit_speedShootingEnergyCost, 25, 0, "Energy cost when firing a weapon while using speed mode");

	pConsole->Register("server_suit_strengthMeleeEnergyCost", &server_suit_strengthMeleeEnergyCost, 35, 0, "Energy cost when performing a melee attack while using cloak mode");
	pConsole->Register("server_suit_strengthShootingEnergyCost", &server_suit_strengthShootingEnergyCost, 35, 0, "Energy cost when firing a weapon while using strength mode");

	pConsole->Register("server_lag_resetmovement", &server_lag_resetmovement, 0, 0, "Enables/Disables Resetting movement on network lag");

	// ---------------
	// Commands
	server_mapTitle = pConsole->RegisterString("server_maptitle", "", 0, "Server Map Title");

}

//------------------------------------------------------------------------
void SC_ServerCVars::ReleaseCVars()
{
	// ------------
	IConsole* pConsole = gEnv->pConsole;

	// ------------
	pConsole->UnregisterVariable("Server_LogVerbosity", true);


	// SC TODO: Delete these!!
	pConsole->UnregisterVariable("filesave_debug", true);
	pConsole->UnregisterVariable("astar_use_cache", true);
	pConsole->UnregisterVariable("astar_callsLimit", true);
	pConsole->UnregisterVariable("astar_callsLimitTime", true);
	pConsole->UnregisterVariable("server_hits_useQueue", true);
	pConsole->UnregisterVariable("server_hits_processDead", true);
	pConsole->UnregisterVariable("server_hits_processDeadTime", true);
	pConsole->UnregisterVariable("server_projectiles_ignoreOwnerCollision", true);
	pConsole->UnregisterVariable("server_projectiles_ownerCollisionMaxTime", true);
	pConsole->UnregisterVariable("server_projectiles_spawnBehind", true);
	pConsole->UnregisterVariable("server_explosion_Limiter", true);
	pConsole->UnregisterVariable("server_debug_vehicleAI", true);
	pConsole->UnregisterVariable("server_disable_latencyReset", true);
	pConsole->UnregisterVariable("server_disable_crympcode", true);
	pConsole->UnregisterVariable("server_debug_positions", true);
	pConsole->UnregisterVariable("server_turret_rocketAimRange", true);
	pConsole->UnregisterVariable("server_turrets_rocketTargetProjectiles", true);
	pConsole->UnregisterVariable("server_turrets_rocketAutoAim", true);
	pConsole->UnregisterVariable("server_turrets_targetOnlyTAC", true);
	pConsole->UnregisterVariable("server_turret_debug", true);
	pConsole->UnregisterVariable("server_turret_debug_step", true);
	pConsole->UnregisterVariable("server_ghostbug_fix", true);
	pConsole->UnregisterVariable("server_use_hit_queue", true);
	pConsole->UnregisterVariable("server_use_explosion_queue", true);
	pConsole->UnregisterVariable("server_classic_chat", true);
	pConsole->UnregisterVariable("server_turrets_target_cloaked", true);
	pConsole->UnregisterVariable("server_autodrop_rpg", true);
	pConsole->UnregisterVariable("server_c4_limit", true);
	pConsole->UnregisterVariable("server_allow_c4Hits", true);
	pConsole->UnregisterVariable("server_allow_scan_explosives", true);
	pConsole->UnregisterVariable("server_allow_scan_cloaked", true);
	pConsole->UnregisterVariable("server_c4_stickToPlayers", true);
	pConsole->UnregisterVariable("server_c4_stickToAllSpecies", true);
	pConsole->UnregisterVariable("server_c4_stickLimitOne", true);
	pConsole->UnregisterVariable("server_fix_spectatorDesync", true);
	pConsole->UnregisterVariable("server_spectatorFix_ResetThreshold", true);
	pConsole->UnregisterVariable("server_anticheat_weaponCheckInterval", true);
	pConsole->UnregisterVariable("server_suit_cloakMeleeEnergyCost", true);
	pConsole->UnregisterVariable("server_suit_cloakShootingEnergyCost", true);
	pConsole->UnregisterVariable("server_suit_speedMeleeEnergyCost", true);
	pConsole->UnregisterVariable("server_suit_speedShootingEnergyCost", true);
	pConsole->UnregisterVariable("server_suit_strengthMeleeEnergyCost", true);
	pConsole->UnregisterVariable("server_suit_strengthShootingEnergyCost", true);
	pConsole->UnregisterVariable("server_lag_resetmovement", true);

	//------------
	/*
	for (auto& entry : m_CVarMap)
		gEnv->pConsole->UnregisterVariable(entry.first.c_str(), true);

	m_CVarMap.clear();*/
}


