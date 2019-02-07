#pragma comment ( lib , "legacy_stdio_definitions.lib" ); //VS 2017

#include "Utils/winbase.h"

#include "interface.h"
#include "filesystem.h"
#include "engine/iserverplugin.h"
#include "game/server/iplayerinfo.h"
#include "eiface.h"
#include "igameevents.h"
#include "convar.h"
#include "Color.h"
#include "vstdlib/random.h"
#include "engine/IEngineTrace.h"
#include "tier2/tier2.h"
#include "tier0/memdbgon.h"
#include "ivrenderview.h"
#include "iclient.h"
#include "icliententitylist.h"
#include "engine/ivdebugoverlay.h"
#include "client_class.h"
#include "vgui/ISurface.h"
#include "iclientmode.h"
#include "view_shared.h"
#include "iclientrenderable.h"
#include "model_types.h"
#include "engine/IEngineTrace.h"
#include "trace.h"
#include "vgui/IPanel.h"
#include "mathlib/mathlib.h"

#include "L4DVersusTool.h"
#include "Utils/hooks.h"
#include "Utils/baseEntityOffset.h"
#include "Utils/virtuals.h"
#include "Features/Bunnyhop.h"
#include "Features/CameraHelper.h"
#include "Features/SkeletonESP.h"
#include "Features/HitBoxESP.h"
#include "Features/BoxESP.h"

#include "Steam/steam_api.h"


void MobEsp() {
	int max_entitys = entList->GetMaxEntities();
	IClientEntity *cl_ent;
	C_BaseEntity *Infected_z = NULL;

	for (int i = 0; i < max_entitys; i++) {
		cl_ent = entList->GetClientEntity(i);
		if (cl_ent == NULL || cl_ent->IsDormant()) continue;
		if (cl_ent != NULL) {
			if (!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName, "Infected")) {
				Infected_z = cl_ent->GetBaseEntity();
				if (!Infected_z) continue;
				//DrawSkeleton2D(cl_ent->GetBaseEntity());
				BoxEsp2D(cl_ent->GetBaseEntity(),255,0,0,255);
			}
		}
	}
}

void MobEsp3D() {
	int max_entitys = entList->GetMaxEntities();
	IClientEntity *cl_ent;
	C_BaseEntity *Infected_z = NULL;

	for (int i = 0; i < max_entitys; i++) {
		cl_ent = entList->GetClientEntity(i);
		if (cl_ent == NULL || cl_ent->IsDormant()) continue;
		if (cl_ent != NULL) {
			if (!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName, "Infected")) {
				Infected_z = cl_ent->GetBaseEntity();
				if (!Infected_z) continue;
				//DrawSkeleton3D(cl_ent->GetBaseEntity());
				//DrawClientHitboxes(cl_ent->GetBaseEntity(), 0, 0, 255, 0, 0);
				//BoxEsp3D(cl_ent->GetBaseEntity(), 0, 255, 0, 0);
			}
		}
	}
}


C_BaseEntity *GetLocalPlayer() {
	C_BaseEntity *player;
	player = entList->GetClientEntity(client->GetLocalPlayer())->GetBaseEntity();
	return player;
}

Vector GetPlayerEyes(){
	CBaseEntity *pEntity = NULL;
	Vector eye_position;
	Vector aim_dir;
	QAngle eye_angles;

	eye_position = GetLocalPlayer()->GetOrign();
	client->GetViewAngles(eye_angles);
	AngleVectors(eye_angles, &aim_dir);
	VectorNormalize(aim_dir);
	Vector vec_end = eye_position + aim_dir * 8000;

	return vec_end;
}

void Init() {
	InitHooks();
}

L4DVersusTool::L4DVersusTool()
{
	m_iClientCommandIndex = 0;
}

L4DVersusTool::~L4DVersusTool()
{
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is loaded, load the interface we need from the engine
//---------------------------------------------------------------------------------
bool L4DVersusTool::Load(	CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory )
{
	ConnectTier1Libraries( &interfaceFactory, 1 );
	ConnectTier2Libraries( &interfaceFactory, 1 );

	CreateInterfaceFn clientFactory = (CreateInterfaceFn)GetProcAddress(GetModuleHandleA("client.dll"), "CreateInterface");
	CreateInterfaceFn serverFactory = (CreateInterfaceFn)GetProcAddress(GetModuleHandleA("server.dll"), "CreateInterface");
	CreateInterfaceFn vguimatsurfaceFactory = (CreateInterfaceFn)GetProcAddress(GetModuleHandleA("vguimatsurface.dll"), "CreateInterface");
	CreateInterfaceFn steamclientFactory = (CreateInterfaceFn)GetProcAddress(GetModuleHandleA("steamclient.dll"), "CreateInterface");

	engine = (IVEngineServer*)interfaceFactory("VEngineServer022", NULL);
	gameeventmanager = (IGameEventManager *)interfaceFactory("GAMEEVENTSMANAGER002",NULL);
	helpers = (IServerPluginHelpers*)interfaceFactory("ISERVERPLUGINHELPERS001", NULL);
	enginetrace = (IEngineTrace *)interfaceFactory("EngineTraceServer003",NULL);
	randomStr = (IUniformRandomStream *)interfaceFactory("VEngineRandom001", NULL);
	renderView = (IVRenderView *)interfaceFactory("VEngineRenderView013", NULL);
	entList = (IClientEntityList*)clientFactory("VClientEntityList003", NULL);
	debug = (IVDebugOverlay*)interfaceFactory("VDebugOverlay003", NULL);
	surface = (vgui::ISurface*)interfaceFactory("VGUI_Surface030", NULL); 
	baseclientdll = (IBaseClientDLL*)clientFactory("VClient016", NULL);
	client = (IVEngineClient*)interfaceFactory("VEngineClient013", NULL);
	material = (IMaterialSystem*)interfaceFactory("VMaterialSystem080", NULL);
	studio_render = (IStudioRender*)interfaceFactory("VStudioRender026", NULL);
	traceRay = (IEngineTrace*)interfaceFactory("EngineTraceClient003", NULL);
	panel = (vgui::IPanel*)interfaceFactory("VGUI_Panel009", NULL);
	model_info = (IVModelInfo*)interfaceFactory("VModelInfoClient004", NULL);
	player_manager = (IPlayerInfoManager*)serverFactory("PlayerInfoManager002", NULL);

	ConVar_Register( 0 );	
	Init();
	return true;
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is unloaded (turned off)
//---------------------------------------------------------------------------------
void L4DVersusTool::Unload( void )
{
	//gameeventmanager->RemoveListener( this ); // make sure we are unloaded from the event system
	DeleteHooks();

	ConVar_Unregister( );
	DisconnectTier2Libraries( );
	DisconnectTier1Libraries( );
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is paused (i.e should stop running but isn't unloaded)
//---------------------------------------------------------------------------------
void L4DVersusTool::Pause( void )
{
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is unpaused (i.e should start executing again)
//---------------------------------------------------------------------------------
void L4DVersusTool::UnPause( void )
{
}

//---------------------------------------------------------------------------------
// Purpose: the name of this plugin, returned in "plugin_print" command
//---------------------------------------------------------------------------------
const char *L4DVersusTool::GetPluginDescription( void )
{
	return "L4DVersusTool, Senny 2018";
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void L4DVersusTool::LevelInit( char const *pMapName )
{
	//gameeventmanager->AddListener( this, true );
}

//---------------------------------------------------------------------------------
// Purpose: called on level start, when the server is ready to accept client connections
//		edictCount is the number of entities in the level, clientMax is the max client count
//---------------------------------------------------------------------------------
void L4DVersusTool::ServerActivate( edict_t *pEdictList, int edictCount, int clientMax )
{
}

//---------------------------------------------------------------------------------
// Purpose: called once per server frame, do recurring work here (like checking for timeouts)
//---------------------------------------------------------------------------------
void L4DVersusTool::GameFrame( bool simulating )
{
}

//---------------------------------------------------------------------------------
// Purpose: called on level end (as the server is shutting down or going to a new map)
//---------------------------------------------------------------------------------
void L4DVersusTool::LevelShutdown( void ) // !!!!this can get called multiple times per map change
{
	if (!clientModePtr) {
		clientModePtr = GetIClientMode();
		IClientModeHooks_Init(clientModePtr);
	}

	if (!CameraHelper_newOrign.IsZero()) {
		CameraHelper_newOrign.Zero();
	}
	//gameeventmanager->RemoveListener( this );
}

//---------------------------------------------------------------------------------
// Purpose: called when a client spawns into a server (i.e as they begin to play)
//---------------------------------------------------------------------------------
void L4DVersusTool::ClientActive( edict_t *pEntity )
{
}

//---------------------------------------------------------------------------------
// Purpose: called when a client leaves a server (or is timed out)
//---------------------------------------------------------------------------------
void L4DVersusTool::ClientDisconnect( edict_t *pEntity )
{
}

//---------------------------------------------------------------------------------
// Purpose: called on 
//---------------------------------------------------------------------------------
void L4DVersusTool::ClientPutInServer( edict_t *pEntity, char const *playername )
{
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void L4DVersusTool::SetCommandClient( int index )
{
	m_iClientCommandIndex = index;
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void L4DVersusTool::ClientSettingsChanged( edict_t *pEdict )
{
}

//---------------------------------------------------------------------------------
// Purpose: called when a client joins a server
//---------------------------------------------------------------------------------
PLUGIN_RESULT L4DVersusTool::ClientConnect( bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen )
{
	return PLUGIN_CONTINUE;
}


//---------------------------------------------------------------------------------
// Purpose: called when a client types in a command (only a subset of commands however, not CON_COMMAND's)
//---------------------------------------------------------------------------------
PLUGIN_RESULT L4DVersusTool::ClientCommand( edict_t *pEntity, const CCommand &args )
{
	return PLUGIN_CONTINUE;
}

//---------------------------------------------------------------------------------
// Purpose: called when a client is authenticated
//---------------------------------------------------------------------------------
PLUGIN_RESULT L4DVersusTool::NetworkIDValidated( const char *pszUserName, const char *pszNetworkID )
{
	return PLUGIN_CONTINUE;
}

//---------------------------------------------------------------------------------
// Purpose: called when a cvar value query is finished
//---------------------------------------------------------------------------------
void L4DVersusTool::OnQueryCvarValueFinished( QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue )
{
}

//---------------------------------------------------------------------------------
// Purpose: called when an event is fired
//---------------------------------------------------------------------------------
void L4DVersusTool::FireGameEvent( KeyValues * event )
{
}

void L4DVersusTool::OnBeginFrame() {
	if (IsDEBUG) {
		Msg("L4DVersusTool::OnBeginFrame()\n");
	}

	if (L4DTestVar2.GetBool()) {
		MobEsp3D();
	}
}

void L4DVersusTool::OnD3D9EndScene() {
	if (IsDEBUG) {
		Msg("L4DVersusTool::OnD3D9EndScene()\n");
	}

	if (L4DTestVar2.GetBool()) {
	}
}

void L4DVersusTool::OnCreateMove(float SampleTime, CUserCmd* cmd) {
	if (IsDEBUG) {
		Msg("L4DVersusTool::OnCreateMove()\n");
	}

	if (L4DBunnyHop.GetBool()) {
		Bunnyhop(cmd);
	}

	if (b_PlayerStuck) {
		cmd->forwardmove = 0;
		cmd->sidemove = 0;
		cmd->upmove = 0;
	}
}

void L4DVersusTool::ONOverrideView(CViewSetup *pSetup) {
	if (IsDEBUG) {
		Msg("L4DVersusTool::ONOverrideView()\n");
	}

	if (L4DCameraHelper.GetBool()) {
		CameraHelper(pSetup);
	}

}

void L4DVersusTool::ONPaintTraverse() {
	if (IsDEBUG) {
		Msg("ONPaintTraverse!\n");
	}

	if (L4DTestVar.GetBool()) {
		MobEsp();
	}
}

//---------------------------------------------------------------------------------
// Purpose: an example of how to implement a new command
//---------------------------------------------------------------------------------
CON_COMMAND( l4d_versusTool, "Prints the version of the plugin." )
{
	Msg( "Version: 2.0\n" );
}

CON_COMMAND(l4d_showInterfaces, "Show the interfaces.")
{
	Msg("IVEngineServer: %x\n", engine);
	Msg("IGameEventManager: %x\n", gameeventmanager);
	Msg("IServerPluginHelpers: %x\n", helpers);
	Msg("IEngineTrace: %x\n", enginetrace);
	Msg("IUniformRandomStream: %x\n", randomStr);
	Msg("CGlobalVars: %x\n", gpGlobals);
	Msg("IVRenderView: %x\n", renderView);
	Msg("IClientEntityList: %x\n", entList);
	Msg("IVDebugOverlay: %x\n", debug);
	Msg("vgui::ISurface: %x\n", surface);
	Msg("IBaseClientDLL: %x\n", baseclientdll);
	Msg("IVEngineClient: %x\n", client);
	Msg("IMaterialSystem: %x\n", material);
	Msg("IStudioRender: %x\n", studio_render);
	Msg("IEngineTrace: %x\n", traceRay);
	Msg("IPanel: %x\n", panel);
	Msg("IVModelInfo: %x\n", model_info);
	Msg("IPlayerInfoManager: %x\n", player_manager);
}

CON_COMMAND(l4d_remove_convar_flags, "Remove conVar flags.") {
	if (strlen(args.Arg(1)) > 0) {
		ConVar * var = NULL;
		var = g_pCVar->FindVar(args[1]);

		if (var == NULL) {
			Warning("Failed to find %s conVar!\n", args[1]);
			return;
		}

		var->RemoveFlags(var->GetFlags());
		Msg("Flags removed!\n");
	}
}



CON_COMMAND(l4d_test, "Test.")
{
	//Msg("%x\n", GetIClientMode());
	//Msg("%d\n", client->IsInGame());
	//Msg("%d\n", client->IsConnected());
	//Msg("%d\n", renderView->GetViewEntity());
	//Msg("%d\n", );

	//int *m_isGhost = (int*)((DWORD)GetLocalPlayer()->GetBaseEntity() + (DWORD)0x196F);
	//*m_isGhost = 1;	
	//Msg("%x\n", steam_client);
}