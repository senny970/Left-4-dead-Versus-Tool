#pragma once
#include "../Utils/winbase.h"
#include "usercmd.h"

enum playercontrols
{
	IN_ATTACK = (1 << 0),
	IN_JUMP = (1 << 1),
	IN_DUCK = (1 << 2),
	IN_FORWARD = (1 << 3),
	IN_BACK = (1 << 4),
	IN_USE = (1 << 5),
	IN_CANCEL = (1 << 6),
	IN_LEFT = (1 << 7),
	IN_RIGHT = (1 << 8),
	IN_MOVELEFT = (1 << 9),
	IN_MOVERIGHT = (1 << 10),
	IN_ATTACK2 = (1 << 11),
	IN_RUN = (1 << 12),
	IN_RELOAD = (1 << 13),
	IN_ALT1 = (1 << 14),
	IN_ALT2 = (1 << 15),
	IN_SCORE = (1 << 16),	// Used by client.dll for when scoreboard is held down
	IN_SPEED = (1 << 17),	// Player is holding the speed key
	IN_WALK = (1 << 18),	// Player holding walk key
	IN_ZOOM = (1 << 19),	// Zoom key for HUD zoom
	IN_WEAPON1 = (1 << 20),	// weapon defines these bits
	IN_WEAPON2 = (1 << 21),	// weapon defines these bits
	IN_BULLRUSH = (1 << 22),
};


void Bunnyhop(CUserCmd * cmd)
{
	if (client->IsInGame() && client->IsConnected()) {
		C_BaseEntity* pLocalPlayer = (C_BaseEntity*)entList->GetClientEntity(client->GetLocalPlayer());
		if (!pLocalPlayer || !pLocalPlayer->IsAlive())
			return;

		auto nFlags = pLocalPlayer->GetFlag();

		if (cmd->buttons & IN_JUMP && GetAsyncKeyState(VK_SPACE)) {			
			if (!(nFlags & FL_ONGROUND))
				cmd->buttons &= ~IN_JUMP;
		}
	}
}