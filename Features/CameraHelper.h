#pragma once
#include "../Utils/winbase.h"
#include "../L4DVersusTool.h"

bool b_PlayerStuck = false;
Vector CameraHelper_newOrign;

void CameraHelper(CViewSetup *pSetup) {
		if (GetAsyncKeyState(VK_CONTROL)) {
			b_PlayerStuck = true;

			unsigned int fSpeed = 5.f;

			if (GetAsyncKeyState(VK_SHIFT))
				fSpeed = fSpeed * 1.65;

			if (GetAsyncKeyState(VK_SPACE))
				fSpeed = fSpeed * 0.45;

			if (CameraHelper_newOrign.IsZero()) {
				CameraHelper_newOrign = pSetup->origin;
			}

			Vector pVecForward, pVecRight, pVecUp;
			AngleVectors(pSetup->angles, &pVecForward, &pVecRight, &pVecUp);

			if (GetAsyncKeyState(0x57)) // W
			{
				CameraHelper_newOrign += pVecForward * fSpeed;
			}

			if (GetAsyncKeyState(0x41)) // A
			{
				CameraHelper_newOrign -= pVecRight * fSpeed;
			}

			if (GetAsyncKeyState(0x53)) // S
			{
				CameraHelper_newOrign -= pVecForward * fSpeed;
			}

			if (GetAsyncKeyState(0x44)) // D
			{
				CameraHelper_newOrign += pVecRight * fSpeed;
			}

			pSetup->origin = CameraHelper_newOrign;		
		}

		b_PlayerStuck = false;
}