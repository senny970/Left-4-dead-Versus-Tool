#pragma once
#include "vmthook.h"
#include "findPattern.h"
#include "../L4DVersusTool.h"
#include "usercmd.h"
#include <d3d9.h>

IDirect3DDevice9* d3d9_device = NULL;

std::unique_ptr<VMTHook> RenderView;
std::unique_ptr<VMTHook> D3D9;
std::unique_ptr<VMTHook> ClientMode;
std::unique_ptr<VMTHook> Panel;

typedef void(__stdcall *EngineStats_BeginFrame_t)(void);
typedef HRESULT(__stdcall *EndScene_t) (IDirect3DDevice9*);
typedef HRESULT(__stdcall *Reset_t) (IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
typedef void(__stdcall *DoPostScreenSpaceEffects_t)(const CViewSetup *pSetup);
typedef void(__thiscall* PaintTraverse_Hooked_t)(vgui::IPanel*, std::uint32_t, bool, bool);

void __stdcall EngineStats_BeginFrame_Hooked(void)
{
	static EngineStats_BeginFrame_t oEngineStats_BeginFrame = RenderView->GetOriginalFunction<EngineStats_BeginFrame_t>(8);
	g_L4DVersusTool.OnBeginFrame();
	oEngineStats_BeginFrame();	
}

HRESULT __stdcall hkEndScene(IDirect3DDevice9* pDevice) {
	static EndScene_t oEndScene = D3D9->GetOriginalFunction<EndScene_t>(42);
	//D3DRECT Brect = { 100,100,200,200 };
	//pDevice->Clear(1, &Brect, D3DCLEAR_TARGET, D3DCOLOR_ARGB(1, 1, 1, 1), 0.0f, 0);
	g_L4DVersusTool.OnD3D9EndScene();
	return oEndScene(pDevice);
}

HRESULT __stdcall hkReset(IDirect3DDevice9* thisptr, D3DPRESENT_PARAMETERS* params) {
	// Get the original function and store it in a static variable for later usage.
	static Reset_t oReset = D3D9->GetOriginalFunction<Reset_t>(16);
	HRESULT result = oReset(thisptr, params);
	return result;
}

bool __stdcall CreateMove_Hooked(float SampleTime, CUserCmd* UserCmd)
{
	if (!UserCmd->command_number)
		return true;

	/*if (EngineClient->IsInGame() && EngineClient->IsConnected())
	{

	}*/
	//Msg("%d", UserCmd->command_number);

	g_L4DVersusTool.OnCreateMove(SampleTime, UserCmd);
	return false;
}

void __stdcall	OverrideView_Hooked(CViewSetup *pSetup) {
	g_L4DVersusTool.ONOverrideView(pSetup);
}

void __stdcall DoPostScreenSpaceEffects_Hooked(const CViewSetup *pSetup) {
	DoPostScreenSpaceEffects_t oDoPostScreenSpaceEffects = ClientMode->GetOriginalFunction<DoPostScreenSpaceEffects_t>(35);
	oDoPostScreenSpaceEffects(pSetup);
}

void __stdcall PaintTraverse_Hooked(std::uint32_t nPanel, bool bForceRepaint, bool bAllowForce)
{
	static PaintTraverse_Hooked_t oPaintTraverse = Panel->GetOriginalFunction<PaintTraverse_Hooked_t>(41);
	oPaintTraverse(panel, nPanel, bForceRepaint, bAllowForce);

	static std::uint32_t nDrawPanel = 0;

	if (!nDrawPanel)
	{
		if (!strcmp(GetPanelName(panel, nPanel), "Panel"))
			nDrawPanel = nPanel;
		else
			return;
	}

	if (nPanel != nDrawPanel)
		return;

	if (!client->IsInGame() || !client->IsConnected())
		return;

	g_L4DVersusTool.ONPaintTraverse();
}

IDirect3DDevice9* GetD3DDevice() {
	typedef IDirect3DDevice9* (*FuncType)(void);
	FuncType FunctionAddress;
	//D3DDevice(void);
	FunctionAddress = (FuncType)FindPattern("shaderapidx9.dll", "\xB8\x00\x00\x00\x00\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x56\x8B\xF1\xE8", "x????xxxxxxxxxxxxxxx");
	if (FunctionAddress) {
		return FunctionAddress();
	}
}

IClientMode* GetIClientMode() {
	typedef IClientMode* (*FuncType)(void);
	FuncType FunctionAddress;
	//GetClientMode(void);
	FunctionAddress = (FuncType)FindPattern("client.dll", "\x8B\x00\x00\x00\x00\x00\x8B\x01\x8B\x00\x00\x00\x00\x00\xFF\xD2\x8B\x00\x00\x00\x00\x00\x00\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xA1\x00\x00\x00\x00\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xB8", "x?????xxx?????xxx??????xxxxxxxxxx????xxxxxxxxxxxx");
	if (FunctionAddress) {
		return FunctionAddress();
	}
}

void InitHooks() {
	RenderView = std::make_unique<VMTHook>(renderView);
	RenderView->HookFunction(EngineStats_BeginFrame_Hooked, 8);

	d3d9_device = *reinterpret_cast<IDirect3DDevice9**>(GetD3DDevice());
	D3D9 = std::make_unique<VMTHook>(d3d9_device);
	D3D9->HookFunction(hkReset, 16);
	D3D9->HookFunction(hkEndScene, 42);

	Panel = std::make_unique<VMTHook>(panel);
	Panel->HookFunction(PaintTraverse_Hooked, 41);
}

void IClientModeHooks_Init(IClientMode *iClientModePtr) {
	ClientMode = std::make_unique<VMTHook>(iClientModePtr);
	ClientMode->HookFunction(CreateMove_Hooked, 25);
	ClientMode->HookFunction(OverrideView_Hooked, 19);
	ClientMode->HookFunction(DoPostScreenSpaceEffects_Hooked, 35);
}

void DeleteHooks() {
	RenderView->UnhookFunction(8);
	D3D9->UnhookFunction(16);
	D3D9->UnhookFunction(42);
	ClientMode->UnhookFunction(19);
	ClientMode->UnhookFunction(25);
	ClientMode->UnhookFunction(35);
}

/*class ISteamClient;
ISteamClient *GetSteamUser() {
	ISteamClient *user;
	typedef ISteamClient*(*addr)();
	addr getsteamuser;
	getsteamuser = (addr)GetProcAddress(GetModuleHandleA("steam_api.dll"), "SteamClient");
	user = getsteamuser();
	return user;
}*/