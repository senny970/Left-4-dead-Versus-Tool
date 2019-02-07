#include "../Utils/virtuals.h"
#include "Color.h"
#include "view_shared.h"
#include "../L4DVersusTool.h"
#include <WinBase.h>

class ISurfaceFix
{
public:
	/*inline void DrawSetColor(Color col)
	{
		//return GetFunction<void(__thiscall*)(ISurfaceFix*, Color)>(this, 11)(this, col);
	}*/

	inline void DrawSetColor(Color col) {
		return GetVirtualFunction<void(__thiscall *)(ISurfaceFix*, Color)>(this, 15)(this, col);
	}

	inline void DrawLine(int x0, int y0, int x1, int y1) {
		return GetVirtualFunction<void(__thiscall *)(ISurfaceFix*, int, int, int, int)>(this, 20)(this, x0, y0, x1, y1);
	}

};

class IVRenderViewFix
{
public:
	inline int GetViewEntity() {
		return GetVirtualFunction<int(__thiscall *)(IVRenderViewFix*)>(this, 31)(this);
	}
};

class IClientRenderableFix
{
public:
	virtual ~IClientRenderableFix() {};

	inline model_t* GetModel()
	{
		return GetVirtualFunction<model_t*(__thiscall*)(IClientRenderableFix*)>(this, 8)(this);
	}

	inline int DrawModel(int flags, float alpha = 1.f)
	{
		return GetVirtualFunction<int(__thiscall*)(IClientRenderableFix*, int, float)>(this, 9)(this, flags, alpha);
	}

	inline bool SetupBones(matrix3x4_t* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime)
	{
		return GetVirtualFunction<bool(__thiscall*)(IClientRenderableFix*, matrix3x4_t*, int, int, float)>(this, 13)(this, pBoneToWorldOut, nMaxBones, boneMask, currentTime);
	}
};

class IMatRenderContextFix {
	public:
		inline void GetViewport(int& x, int& y, int& width, int& height) {
			GetVirtualFunction<void(__thiscall*)(IMatRenderContextFix*, int&, int&, int&, int&)>(this, 39)(this, x, y, width, height);
		}
};

class IMaterialSystemFix {
	public:
		inline IMatRenderContext* GetRenderContext() {
			return GetVirtualFunction<IMatRenderContext*(__thiscall*)(IMaterialSystemFix*)>(this, 95)(this);
		}
};

class IPanelFix
{
public:
	const char* GetName(std::uint32_t iPanel)
	{
		return GetVirtualFunction<const char*(__thiscall*)(IPanelFix*, std::uint32_t)>(this, 37)(this, iPanel);
	}
};


//------------------------------------------Functions-------------------------------------------//

void DrawEntityModel(IClientRenderable* entityRenderable, int flags, float alpha = 1.f) {
	IClientRenderableFix *fix_class = (IClientRenderableFix*)entityRenderable;
	fix_class->DrawModel(flags, alpha);
}

const model_t* GetEntityModel(IClientRenderable* entityRenderable) {
	IClientRenderableFix *fix_class = (IClientRenderableFix*)entityRenderable;
	return fix_class->GetModel();
}

IMatRenderContext* GetRenderContext(IMaterialSystem * material) {
	IMaterialSystemFix *mat = (IMaterialSystemFix*)material;
	return mat->GetRenderContext();
}

void GetViewport(IMaterialSystem * material, int& x, int& y, int& width, int& height) {
	IMatRenderContextFix *ctx = (IMatRenderContextFix*)GetRenderContext(material);
	ctx->GetViewport(x, y, width, height);
}

const char *GetPanelName(vgui::IPanel* panel, std::uint32_t iPanel) {
	IPanelFix *pnl = (IPanelFix*)panel;
	return pnl->GetName(iPanel);
}

bool SetupBonesEntity(IClientRenderable *renderable, matrix3x4_t* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime) {
	IClientRenderableFix *render = (IClientRenderableFix*)renderable;
	return render->SetupBones(pBoneToWorldOut, nMaxBones, boneMask, currentTime);
}