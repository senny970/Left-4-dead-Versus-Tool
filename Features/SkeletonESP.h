#pragma once
#include "../L4DVersusTool.h"

void DrawSkeleton3D(C_BaseEntity* pEntity)
{
	QAngle BoneAng, BoneParAng;
	Vector BonePos, BoneParPos;

	studiohdr_t* StudioModel = model_info->GetStudiomodel(GetEntityModel(pEntity->GetClientRenderable()));
	if (!StudioModel)
		return;

	static matrix3x4_t matrix[128];
	if (!SetupBonesEntity(pEntity->GetClientRenderable(), matrix, 128, 0x100, player_manager->GetGlobalVars()->curtime))
		return;

	for (std::uint32_t i = 0; i < StudioModel->numbones; i++)
	{
		auto m_Bone = StudioModel->pBone(i);
		if (!m_Bone || !(m_Bone->flags & 256) || m_Bone->parent == -1)
			continue;

		debug->AddCoordFrameOverlay(matrix[i], 3.0f);
	}
}

void DrawSkeleton3D_Lines(C_BaseEntity* pEntity)
{
	QAngle BoneAng, BoneParAng;
	Vector BonePos, BoneParPos;

	studiohdr_t* StudioModel = model_info->GetStudiomodel(GetEntityModel(pEntity->GetClientRenderable()));
	if (!StudioModel)
		return;

	static matrix3x4_t matrix[128];
	if (!SetupBonesEntity(pEntity->GetClientRenderable(), matrix, 128, 0x100, player_manager->GetGlobalVars()->curtime))
		return;

	for (std::uint32_t i = 0; i < StudioModel->numbones; i++)
	{
		auto m_Bone = StudioModel->pBone(i);
		if (!m_Bone || !(m_Bone->flags & 256) || m_Bone->parent == -1)
			continue;

		MatrixAngles(matrix[i], BoneAng, BonePos);
		MatrixAngles(matrix[m_Bone->parent], BoneParAng, BoneParPos);
		debug->AddLineOverlay(BonePos, BoneParPos, 0, 255, 0, 1, 0);
	}
}

void DrawSkeleton2D(C_BaseEntity* pEntity)
{
	studiohdr_t* StudioModel = model_info->GetStudiomodel(GetEntityModel(pEntity->GetClientRenderable()));
	if (!StudioModel)
		return;

	static matrix3x4_t matrix[128];
	if (!SetupBonesEntity(pEntity->GetClientRenderable(), matrix, 128, 0x100, player_manager->GetGlobalVars()->curtime))
		return;

	surface->DrawSetColor(0, 255, 0, 255);

	for (std::uint32_t i = 0; i < StudioModel->numbones; i++)
	{
		auto m_Bone = StudioModel->pBone(i);
		if (!m_Bone || !(m_Bone->flags & 256) || m_Bone->parent == -1)
			continue;

		Vector vecBonePos;
		debug->ScreenPosition( Vector(matrix[i][0][3], matrix[i][1][3], matrix[i][2][3]), vecBonePos);

		Vector vecBoneParent;
		debug->ScreenPosition(Vector(matrix[m_Bone->parent][0][3], matrix[m_Bone->parent][1][3], matrix[m_Bone->parent][2][3]), vecBoneParent);


		surface->DrawLine(vecBonePos.x, vecBonePos.y, vecBoneParent.x, vecBoneParent.y);
	}
}

//Original
/*void DrawSkeleton(C_BaseEntity* pEntity)
{
	studiohdr_t* StudioModel = model_info->GetStudiomodel(GetEntityModel(pEntity->GetClientRenderable()));
	if (!StudioModel)
		return;

	static matrix3x4_t matrix[128];
	if (!SetupBonesEntity(pEntity->GetClientRenderable(), matrix, 128, 0x100, player_manager->GetGlobalVars()->curtime))
		return;

	surface->DrawSetColor(255, 255, 255, 255);

	for (std::uint32_t i = 0; i < StudioModel->numbones; i++)
	{
		auto m_Bone = StudioModel->pBone(i);
		if (!m_Bone || !(m_Bone->flags & 256) || m_Bone->parent == -1)
			continue;

		Vector vecBonePos;
		if (!WorldToScreen(Vector(matrix[i][0][3], matrix[i][1][3], matrix[i][2][3]), vecBonePos))
			continue;

		Vector vecBoneParent;
		if (!WorldToScreen(Vector(matrix[m_Bone->parent][0][3], matrix[m_Bone->parent][1][3], matrix[m_Bone->parent][2][3]), vecBoneParent))
			continue;

		surface->DrawLine(vecBonePos.x, vecBonePos.y, vecBoneParent.x, vecBoneParent.y);
	}
}*/
