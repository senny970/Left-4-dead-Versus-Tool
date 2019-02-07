#pragma once
#include "Utils/SDKfix.h"
#include "cdll_int.h"

// Interfaces from the engine
IVEngineServer	*engine = NULL; // helper functions (messaging clients, loading content, making entities, running commands, etc)
IGameEventManager *gameeventmanager = NULL; // game events interface
IPlayerInfoManager *playerinfomanager = NULL; // game dll interface to interact with players
IBotManager *botmanager = NULL; // game dll interface to interact with bots
IServerPluginHelpers *helpers = NULL; // special 3rd party plugin helpers from the engine
IUniformRandomStream *randomStr = NULL;
IEngineTrace *enginetrace = NULL;
CGlobalVars *gpGlobals = NULL;
IVRenderView  *renderView = NULL;
IClientEntityList *entList = NULL;
IVDebugOverlay *debug = NULL;
vgui::ISurface *surface = NULL;
IBaseClientDLL *baseclientdll = NULL;
IVEngineClient *client = NULL;
IMaterialSystem * material = NULL;
IStudioRender *studio_render = NULL;
IEngineTrace* traceRay = NULL;
vgui::IPanel* panel;
IVModelInfo *model_info = NULL;
IPlayerInfoManager *player_manager = NULL;

IClientMode *clientModePtr = NULL;

ConVar L4DTestVar("l4d_testvar", "0", 0, "Test var.\n");
ConVar L4DBunnyHop("l4d_bunnyhop_enable", "0", 0, "Enable BunnyHop.\n");
ConVar L4DDebugMode("l4d_debug_mode", "0", 0, "Enable Debuging.\n");
ConVar L4DTestVar2("l4d_testvar2", "0", 0, "Test var2.\n");
ConVar L4DHeadPosX("l4d_HeadposX", "0", 0, "Test var2.\n");
ConVar L4DHeadPosY("l4d_HeadposY", "0", 0, "Test var2.\n");
ConVar L4DHeadPosZ("l4d_HeadposZ", "70", 0, "Test var2.\n");
ConVar L4DCameraHelper("l4d_camera_helper", "0", 0, "Camera Helper.\n");
#define IsDEBUG L4DDebugMode.GetBool()

class L4DVersusTool: public IServerPluginCallbacks, public IGameEventListener
{
public:
	L4DVersusTool();
	~L4DVersusTool();

	// IServerPluginCallbacks methods
	virtual bool			Load(	CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory );
	virtual void			Unload( void );
	virtual void			Pause( void );
	virtual void			UnPause( void );
	virtual const char     *GetPluginDescription( void );
	virtual void			LevelInit( char const *pMapName );
	virtual void			ServerActivate( edict_t *pEdictList, int edictCount, int clientMax );
	virtual void			GameFrame( bool simulating );
	virtual void			LevelShutdown( void );
	virtual void			ClientActive( edict_t *pEntity );
	virtual void			ClientDisconnect( edict_t *pEntity );
	virtual void			ClientPutInServer( edict_t *pEntity, char const *playername );
	virtual void			SetCommandClient( int index );
	virtual void			ClientSettingsChanged( edict_t *pEdict );
	virtual PLUGIN_RESULT	ClientConnect( bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen );
	virtual PLUGIN_RESULT	ClientCommand( edict_t *pEntity, const CCommand &args );
	virtual PLUGIN_RESULT	NetworkIDValidated( const char *pszUserName, const char *pszNetworkID );
	virtual void			OnQueryCvarValueFinished( QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue );
	virtual void FireGameEvent( KeyValues * event );
	virtual int GetCommandIndex() { return m_iClientCommandIndex; }

	void OnBeginFrame();
	void OnD3D9EndScene();
	void OnCreateMove(float SampleTime, CUserCmd* cmd);
	void ONOverrideView(CViewSetup *pSetup);
	void ONPaintTraverse();
private:
	int m_iClientCommandIndex;
};

L4DVersusTool g_L4DVersusTool;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(L4DVersusTool, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_L4DVersusTool);