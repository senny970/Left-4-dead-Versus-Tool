#pragma once
#include "../L4DVersusTool.h"

static Vector	hullcolor[8] =
{
	Vector(1.0, 1.0, 1.0),
	Vector(1.0, 0.5, 0.5),
	Vector(0.5, 1.0, 0.5),
	Vector(1.0, 1.0, 0.5),
	Vector(0.5, 0.5, 1.0),
	Vector(1.0, 0.5, 1.0),
	Vector(0.5, 1.0, 1.0),
	Vector(1.0, 1.0, 1.0)
};

void DrawClientHitboxes(C_BaseEntity* pEntity, float duration, int r = 0, int g = 0, int b = 0, int a = 0)
{
	studiohdr_t* StudioModel = model_info->GetStudiomodel(GetEntityModel(pEntity->GetClientRenderable()));
	if (!StudioModel)
		return;

	static matrix3x4_t matrix[128];
	if (!SetupBonesEntity(pEntity->GetClientRenderable(), matrix, 128, 0x100, player_manager->GetGlobalVars()->curtime))
		return;

	Vector position;
	QAngle angles;

	int in_r = 255;
	int in_g = 0;
	int in_b = 0;

	mstudiohitboxset_t *set = StudioModel->pHitboxSet(0);
	int HitboxCount = set->numhitboxes;

	for (int i = 0; i < HitboxCount; i++) {

		mstudiobbox_t *pbox = StudioModel->pHitbox(i, 0);
		MatrixAngles(matrix[pbox->bone], angles, position);

		if ((r + g + b) > 0) {
			debug->AddBoxOverlay(position, pbox->bbmin, pbox->bbmax, angles, r, g, b, a, duration);		
			//debug->AddTextOverlay(position, 0, "%d\n", i);
		}
		else {
			int j = (pbox->group % 8);
			in_r = (int)(255.0f * hullcolor[j][0]);
			in_g = (int)(255.0f * hullcolor[j][1]);
			in_b = (int)(255.0f * hullcolor[j][2]);
			debug->AddBoxOverlay(position, pbox->bbmin, pbox->bbmax, angles, in_r, in_g, in_b, a, duration);
		}
	}
}