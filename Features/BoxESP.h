#pragma once
#include "../L4DVersusTool.h"


Vector GetHitbox(C_BaseEntity * pEntity, int nHitbox)
{
	matrix3x4_t matrix[128];

	if (SetupBonesEntity(pEntity->GetClientRenderable(), matrix, 128, 0x100, 0))
	{
		studiohdr_t* hdr = model_info->GetStudiomodel(GetEntityModel(pEntity->GetClientRenderable()));
		mstudiohitboxset_t* set = hdr->pHitboxSet(0);
		mstudiobbox_t* hitbox = set->pHitbox(nHitbox);

		if (hitbox)
		{
			Vector vMin, vMax, vCenter;
			VectorTransform(hitbox->bbmin, matrix[hitbox->bone], vMin);
			VectorTransform(hitbox->bbmax, matrix[hitbox->bone], vMax);
			vCenter = ((vMin + vMax) * 0.5f);

			return vCenter;
		}
	}
	return Vector{ 0,0,0 };
}

void BoxEsp2D(C_BaseEntity *entity, int r, int g, int b, int a = 255) {
	int HeadHitBox = 10;
	Vector Origin3D = entity->GetOrign();
	Vector Origin;

	debug->ScreenPosition(Origin3D, Origin);

	Vector Head3D;
	Vector Head;

	/*static matrix3x4_t matrix[128];
	if (!SetupBonesEntity(entity->GetClientRenderable(), matrix, 128, 0x100, player_manager->GetGlobalVars()->curtime))
		return;*/

	Head3D = GetHitbox(entity, HeadHitBox);//Vector(matrix[14][0][3], matrix[14][1][3], matrix[14][2][3]);
	Head3D.z += 15.0f;

	debug->ScreenPosition(Head3D, Head);

	float h = fabs(Head.y - Origin.y);
	float w = h / 1.65f;

	RECT rect = {
		static_cast<long>(Origin.x - w * 0.5f),
		static_cast<long>(Head.y),
		static_cast<long>(w),
		static_cast<long>(Origin.y)
	};

	rect.right += static_cast<long>(rect.left);

	surface->DrawSetColor(r, g, b, a);
	surface->DrawOutlinedRect(rect.left, rect.top, rect.right, rect.bottom);
}

void BoxEsp3D(C_BaseEntity *entity, int r, int g, int b, int a = 255) {
	int HeadHitBox = 10;
	Vector Origin3D = entity->GetOrign();
	Vector Head3D;

	/*static matrix3x4_t matrix[128];
	if (!SetupBonesEntity(entity->GetClientRenderable(), matrix, 128, 0x100, player_manager->GetGlobalVars()->curtime))
		return;*/

	Head3D = GetHitbox(entity, HeadHitBox);//Vector(matrix[14][0][3], matrix[14][1][3], matrix[14][2][3]);
	Head3D.z += 10.0f;

	vec_t BoxZ = Head3D.z - entity->GetOrign().z;

	debug->AddBoxOverlay(entity->GetOrign(), Vector(-15,-15,0), Vector(15,15, BoxZ), QAngle(0, 0, 0), r, g, b, a, 0);	
}