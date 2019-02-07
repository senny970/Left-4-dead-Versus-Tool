#pragma comment ( lib , "legacy_stdio_definitions.lib" ); //VS 2017

#define CLIENT_DLL		
#include <windows.h>
#include "cbase.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <ctime>
#include <direct.h>
#include <windows.h>
#include <io.h>
#include <Psapi.h>

#include "iclient.h"
#include "cdll_int.h"
#include "con_nprint.h"
#include "server_class.h"

#include <strsafe.h>
#include <string>

#include "toolframework/ienginetool.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterialvar.h"
#include "engine/ivdebugoverlay.h"
#include "vphysics_interface.h"
#include "c_baseanimating.h"
#include "bone_setup.h"
#include "studio.h"
#include "ISurface.h"
//#include "ISurfaceV30.h"

//Custom headers
#include "convar_sm_l4d.h"
#include "FindPattern.h"

#include "interface.h"
#include "filesystem.h"
#include "engine/iserverplugin.h"
#include "game/server/iplayerinfo.h"
#include "eiface.h"
#include "igameevents.h"
#include "convar.h"
#include "Color.h"
#include "vstdlib/random.h"
#include "vstdlib/jobthread.h"
#include "engine/IEngineTrace.h"
#include "tier2/tier2.h"
#include "tier1/iconvar.h"
#include "tier1/convar.h"
#include "tier0/memdbgon.h"

#include "view_shared.h"
#include "iviewRender.h"

#include "iclientmode.h"

#include "Open Steamworks/ISteamGameServer009.h"

#define SPACE_BAR 0x20

#define ZC_SMOKER 1
#define ZC_BOOMER 2
#define ZC_HUNTER 3
#define ZC_WITCH 4
#define ZC_TANK 5

#define Spectator 1
#define Survivor 2
#define Infected 3

#define Pistol 1
#define Smg 2
#define PumpShotgun 3
#define AutoShotgun 4
#define Rife 5
#define HuntingRife 6
#define MedKit 8
#define Molotov 9
#define PipeBomb 10
#define PainPills 12

#define	MAX_OVERLAY_DIST_SQR 90000000

int gg = 0;
int plus = 0;

//-------------------------------------------------------/Signatures/---------------------------------------//
/*engine.dll - IMaterial *GetMaterialAtCrossHair(void);
83EC 18       SUB ESP,18
8B0D 605CF808 MOV ECX,DWORD PTR DS:[8F85C60]
8B01          MOV EAX,DWORD PTR DS:[ECX]
8B50 14       MOV EDX,DWORD PTR DS:[EAX+14]
56            PUSH ESI
FFD2          CALL EDX*/
#define GetMaterialAtCrossHair_SIG "\x83\xEC\x18\x8B\x0D\x00\x00\x00\x00\x8B\x01\x8B\x50\x14\x56\xFF\xD2"
#define GetMaterialAtCrossHair_MASK "xxxxx????xxxxxxxx"

//client.dll - void C_BaseAnimating::DrawSkeleton( CStudioHdr const* pHdr, int iBoneMask ) const
#define DrawSkeleton_SIG "\x83\xEC\x18\x53\x8B\x5C\x24\x20\x85\xDB\x55\x8B\xE9"
#define DrawSkeleton_MASK "xxxxxxxxxxxxx"
/*call
143CFA48   . /74 2E         JE SHORT client.143CFA78
143CFA4A   . |8B4D 10       MOV ECX,DWORD PTR SS:[EBP+10]
143CFA4D   . |51            PUSH ECX
143CFA4E   . |53            PUSH EBX
143CFA4F   . |8BCF          MOV ECX,EDI
143CFA51   . |E8 1A8AFFFF   CALL client.143C8470*/


/*client.dll - bool C_BaseAnimating::SetupBones( matrix3x4a_t *pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime )
138EF3A0   $  55            PUSH EBP
138EF3A1   .  8BEC          MOV EBP,ESP
138EF3A3   .  83E4 F0       AND ESP,FFFFFFF0
138EF3A6   .  81EC 940E0000 SUB ESP,0E94
138EF3AC   .  803D 75B3DE13>CMP BYTE PTR DS:[13DEB375],0
138EF3B3   .  53            PUSH EBX
138EF3B4   .  56            PUSH ESI
138EF3B5   .  57            PUSH EDI
138EF3B6   .  8BF1          MOV ESI,ECX
138EF3B8   .  B8 7C86C713   MOV EAX,client.13C7867C                  ;  ASCII "Client_Animation"
138EF3BD   .  74 05         JE SHORT client.138EF3C4
138EF3BF   .  B8 E48FC713   MOV EAX,client.13C78FE4                  ;  ASCII "Client_Animation_Threaded"
138EF3C4   >  8B0D 7402C613 MOV ECX,DWORD PTR DS:[<&tier0.g_VProfCur>;  tier0.g_VProfCurrentProfile
138EF3CA   .  6A 04         PUSH 4
138EF3CC   .  6A 00         PUSH 0
138EF3CE   .  50            PUSH EAX
138EF3CF   .  6A 00         PUSH 0
138EF3D1   .  68 C88FC713   PUSH client.13C78FC8                     ;  ASCII "C_BaseAnimating::SetupBones"
138EF3D6   .  FF15 7802C613 CALL DWORD PTR DS:[<&tier0.?EnterScope@C>;  tier0.?EnterScope@CVProfile@@QAEXPBDH0_NH@Z
138EF3DC   .  8D7E FC       LEA EDI,DWORD PTR DS:[ESI-4]
138EF3DF   .  FF15 3402C613 CALL DWORD PTR DS:[<&tier0.ThreadInMainT>;  tier0.ThreadInMainThread*/
#define SetupBones_SIG "\x6A\x04\x6A\x00\x50\x6A\x00\x68\x00\x00\x00\x00\xFF\x15\x00\x00\x00\x00\x8D\x7E\xFC\xFF\x15\x00\x00\x00\x00"
#define SetupBones_MASK "xxxxxxxx????xx????xxxxx????" //FindPattern()+(DWORD)A2; or Dec(42)

/*C_BaseAnimating::DrawClientHitboxes(float duration, bool monocolor)
//this = ecx
1453B6C0  /$  83EC 30                      SUB ESP,30
1453B6C3  |.  55                           PUSH EBP
1453B6C4  |.  57                           PUSH EDI
1453B6C5  |.  8BF9                         MOV EDI,ECX
1453B6C7  |.  33ED                         XOR EBP,EBP
1453B6C9  |.  39AF 64010000                CMP DWORD PTR DS:[EDI+164],EBP
1453B6CF  |.  0F8E 8B010000                JLE client.1453B860*/
#define DrawClientHitboxes_SIG "\x83\xEC\x30\x55\x57\x8B\xF9\x33\xED\x39\xAF\x00\x00\x00\x00\x0F\x8E\x00\x00\x00\x00"
#define DrawClientHitboxes_MASK "xxxxxxxxxxx????xx????"

//IViewRender *GetViewRenderInstance()
#define GetViewRenderInstance_SIG "\xB8\x00\x00\x00\x00\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xE8\x00\x00\x00\x00\xC6\x80\x40\x28\x00\x00\x01"
#define GetViewRenderInstance_MASK "x????xxxxxxxxxxxx????xxxxxxx"

//IClientMode *GetClientMode();
#define GetClientMode_SIG "\x8B\x0D\x00\x00\x00\x00\x8B\x01\x8B\x90\x00\x00\x00\x00\xFF\xD2\x8B\x04\x85\x00\x00\x00\x00\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xA1\x00\x00\x00\x00\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xB8\x00\x00\x00\x00"
#define GetClientMode_MASK "xx????xxxx????xxxxx????xxxxxxxxxx????xxxxxxxxxxxx????"

IVEngineServer	*engine_sv = NULL;
IVEngineClient	*engine_cl = NULL;
IClientEntityList *entList = NULL;
IVPhysicsDebugOverlay *vphysics = NULL;
//IVEngineServer	*engine = NULL;
IPlayerInfoManager *playerinfomanager = NULL;
CGlobalVars *globals = NULL;
IEngineTrace *enginetrace = NULL;
IVEngineClient *client = NULL;
IEngineTool *ienginetool = NULL;
IBaseClientDLL *clientDLL = NULL;
IServerGameDLL *gamedll = NULL;	
IGameEventManager *cl_gameeventmanager = NULL;
IServerGameEnts *sv_ents = NULL;
IServerGameClients *sv_gcl = NULL;
IServerTools *server_tool = NULL;
IClientTools *client_tool = NULL;
IVDebugOverlay *debug = NULL;
IVModelRender *model_render = NULL;
IVModelInfo *model_info = NULL;
IPhysicsObject *physOBJ = NULL;
IMDLCache *mdl_cache = NULL;
IStudioRender *std_render = NULL;
vgui::ISurface *surface = NULL;
IMDLCache *mdlcache = NULL;

ISteamGameServer009 *gameServer = NULL;

ConVar AutoRecordDemoMp("l4d_autorecord_demo_mp", "0", FCVAR_ARCHIVE, "Auto recording demo in Multyplayer.\n");
ConVar AutoRecordDemoSp("l4d_autorecord_demo_sp", "0", FCVAR_ARCHIVE, "Auto recording demo in Singleplayer.\n");
ConVar SetHudColor_r("l4d_versus_tool_set_hud_color_r", "0.0", FCVAR_ARCHIVE, "Set msg hud color R.\n");
ConVar SetHudColor_g("l4d_versus_tool_set_hud_color_g", "0.5", FCVAR_ARCHIVE, "Set msg hud color G.\n");
ConVar SetHudColor_b("l4d_versus_tool_set_hud_color_b", "0.9", FCVAR_ARCHIVE, "Set msg hud color B.\n");
ConVar SetHudTime_t("l4d_versus_tool_set_hud_time", "5.0", FCVAR_ARCHIVE, "Set msg display time hud on screen.\n");
ConVar L4DEspPZAlpha("l4d_esp_pz_alpha", "5", FCVAR_ARCHIVE, "l4d_esp_pz_alpha.\n");
ConVar L4DEspPZ("l4d_esp_pz", "0", 0, "l4d_esp_pz.\n");
ConVar L4DEspInfected("l4d_esp_infected", "0", 0, "l4d_esp_infected.\n");
ConVar L4DHunterDistanceMeter("l4d_hunter_distance_meter", "0", 0, "l4d_hunter_distance_meter.\n");
ConVar L4DHunterDistanceMeterAngle("l4d_hunter_distance_meter_angle", "-70", FCVAR_ARCHIVE, "l4d_hunter_distance_meter_angle.\n");
ConVar DrawPZGlow("l4d_draw_pz_glow", "0", 0, "l4d_draw_pz_glow.\n");
ConVar DrawViewLine("l4d_draw_view_line", "0", 0, "l4d_draw_view_line.\n");
ConVar L4DAimBotPZ("l4d_aim_bot_pz", "0", 0, "l4d_aim_bot_pz.\n");
ConVar L4DAimBotPZTargetBone("l4d_aim_bot_pz_target_bone", "14", FCVAR_ARCHIVE, "l4d_aim_bot_pz_target_bone.\n");
ConVar L4DAimBotPZUseKey("l4d_aim_use_key", "69", FCVAR_ARCHIVE, "l4d_aim_use_key.\n");
ConVar L4DAimBotPZBreakKey("l4d_aim_break_key", "17", FCVAR_ARCHIVE, "l4d_aim_break_key.\n");
ConVar L4DAimBotPZRadius("l4d_aim_bot_pz_radius", "300", FCVAR_ARCHIVE, "l4d_aim_bot_pz_radius.\n");
ConVar L4DAimBotInfected("l4d_aim_bot_infected", "0", 0, "l4d_aim_bot_infected.\n");
ConVar L4DAimBotInfectedTargetBone("l4d_aim_bot_infected_target_bone", "14", FCVAR_ARCHIVE, "l4d_aim_bot_infected_target_bone.\n");
ConVar L4DAimBotInfectedRadius("l4d_aim_bot_infected_radius", "300", FCVAR_ARCHIVE, "l4d_aim_bot_infected_radius.\n");
ConVar L4DAimBotInfectedOverlayMode("l4d_aim_bot_infected_overlay_mode", "1", FCVAR_ARCHIVE, "l4d_aim_bot_infected_overlay_mode.\n");
ConVar L4DAimBotInfected_CompensationX("l4d_aim_bot_infected_compensation_x", "0", FCVAR_ARCHIVE, "l4d_aim_bot_infected_compensation_x.\n");
ConVar L4DAimBotInfected_CompensationY("l4d_aim_bot_infected_compensation_y", "0", FCVAR_ARCHIVE, "l4d_aim_bot_infected_compensation_y.\n");
ConVar L4DAimBotInfected_CompensationZ("l4d_aim_bot_infected_compensation_z", "0", FCVAR_ARCHIVE, "l4d_aim_bot_infected_compensation_z.\n");
ConVar L4DDrawRay("l4d_draw_ray", "0", FCVAR_ARCHIVE, "l4d_draw_ray.\n");
ConVar DebugGameEvents("l4d_debug_game_events", "0", FCVAR_ARCHIVE ,"Enable debug mode for Game Events.\n");
ConVar ListenChat("l4d_listen_chat", "0", FCVAR_ARCHIVE ,"Enable listen mode for chat.\n");
ConVar KillCounter("l4d_zombie_kill_counter", "0", 0 ,"Enable zombie kills counter.\n");
ConVar L4DBhop("l4d_bhop", "0", 0 ,"Enable Bunny hop.\n");
ConVar ThreadRate("l4d_main_thread_rate", "1", FCVAR_ARCHIVE ,"Set the main thread update rate in (ms).\n");
ConVar ThreadDebug("l4d_main_thread_debug", "0", 0 ,"Debug main thread.\n");
ConVar ThreadDebugRange("l4d_main_thread_debug_value_range", "1000", FCVAR_ARCHIVE ,"l4d_main_thread_debug_value_range.\n");
ConVar ShowItems("l4d_show_items", "0", 0 ,"Show Items on map.\n");
ConVar ShowPlayersHealth("l4d_show_players_health", "0", 0 ,"Show Players health.\n");
ConVar TankHelpers("l4d_tank_helpers", "0", 0 ,"Helpers for tank.\n");
ConVar DrawSkeletonInfected("l4d_draw_skeleton_infected", "0", 0 ,"l4d_draw_skeleton_infected.\n");
ConVar L4DTest2("l4d_test2", "0", 0 ,"l4d_test2.\n");
ConVar L4DTest3("l4d_test3", "8000", 0 ,"l4d_test3.\n");

#define R g_pCVar->FindVar("l4d_versus_tool_set_hud_color_r")->GetFloat()
#define G g_pCVar->FindVar("l4d_versus_tool_set_hud_color_g")->GetFloat()
#define B g_pCVar->FindVar("l4d_versus_tool_set_hud_color_b")->GetFloat()
#define T g_pCVar->FindVar("l4d_versus_tool_set_hud_time")->GetFloat()

char gamedir[MAX_PATH];
char gamedir_demos[MAX_PATH];
int l4d_thirdpersonshoulder_click = 0;
int l4d_versus_thirdperson_click = 0;
int l4d_r_drawothermodels_click = 0;
int l4d_r_drawworld_click = 0;
int l4d_showtriggers_toggle_click = 0;
int l4d_r_drawclipbrushes_click = 0;
int l4d_r_drawstaticprops_click = 0;
int l4d_z_view_distance_click = 0;
int l4d_mat_wireframe_click = 0;
int l4d_cl_viewmodelfovsurvivor_click = 0;
int l4d_cl_viewmodelfovboomer_click = 0;
int l4d_cl_viewmodelfovhunter_click = 0;
int l4d_cl_viewmodelsmoker_click = 0;
int l4d_cl_viewmodeltank_click = 0;
int l4d_host_timescale_click = 0;
int l4d_sv_gravity_click = 0;
int l4d_developer_click = 0;
int l4d_sv_friction_click = 0;
int l4d_vcollide_wireframe_click = 0;
int l4d_slow_motion_click = 0;
int l4d_ignorez_click = 0;
int record = 0;
int rec = 0;
int l4d_test_click = 0;
bool team_state = false;

//Zombie counter
int *m_iTeamNum = 0;
int zombies = 0;
int headshots = 0;
int players = 0;

//Owner AimEntity ID && AimDistance
int EntID = 0;
float Distance = 0;

//Owner AimPos
Vector OwnerAimPos;

//IsVisible
Vector EnemyBone_v;
int MobID = 0;

//AimPointTick
int AimPointTick = 0;

//AimBot Infected
int Distance_own_inf = 0;
int Dis_tmp = 0;
int elem_pos = 0;

//Setup_bones
Vector BonePos;
QAngle BoneAng;
matrix3x4_t bone_matrix[MAXSTUDIOBONES];
int ClientNum = 1;
int BoneNum = 0;

//MainThread
HANDLE l4d_thread;
HANDLE l4d_drawing_thread;
bool isThreadActive = true;			

void r_dravn_box(Vector &vecOrigin)
{
	int r, g, b;
	r = 0;
	g = 255;
	b = 0;

	Vector maxs = Vector(-10.0f, 10.0f, 65.0f);
	Vector mins = Vector(10.0f, -10.0f, 0.0f);


	if( mins.x == maxs.x && mins.y == maxs.y && mins.z == maxs.z )
	{
		mins =  ( -15.0, -15.0, -15.0 );
		maxs =  ( 15.0, 15.0, 15.0 );
	}
	else
	{
		VectorAdd(vecOrigin, maxs, maxs);
		VectorAdd(vecOrigin, mins, mins);

	}
		
	Vector  vPos1,  vPos2,  vPos3,  vPos4,  vPos5,  vPos6;
	vPos1 = maxs;	vPos1.x = mins.x;
	vPos2 = maxs;	vPos2.y = mins.y;
	vPos3 = maxs;	vPos3.z = mins.z;
	vPos4 = mins;	vPos4.x = maxs.x;
	vPos5 = mins;	vPos5.y = maxs.y;
	vPos6 = mins;	vPos6.z = maxs.z;
	

	debug->AddLineOverlay(maxs, vPos1, r, g, b,true , 0.01);
	debug->AddLineOverlay(maxs, vPos2, r, g, b,true , 0.01);	
	debug->AddLineOverlay(maxs, vPos3, r, g, b,true , 0.01);		
	debug->AddLineOverlay(vPos6, vPos1, r, g, b,true , 0.01);	
	debug->AddLineOverlay(vPos6, vPos2, r, g, b,true , 0.01);	
	debug->AddLineOverlay(vPos6, mins, r, g, b,true , 0.01);	
	debug->AddLineOverlay(vPos4, mins, r, g, b,true , 0.01);		
	debug->AddLineOverlay(vPos5, mins, r, g, b,true , 0.01);		
	debug->AddLineOverlay(vPos5, vPos1, r, g, b,true , 0.01);	
	debug->AddLineOverlay(vPos5, vPos3, r, g, b,true , 0.01);		
	debug->AddLineOverlay(vPos4, vPos3, r, g, b,true , 0.01);
	debug->AddLineOverlay(vPos4, vPos2, r, g, b,true , 0.01);
	//VertArrow( vecOrigin+Vector(0,0,80.0f), vecOrigin+Vector(0,0,190.0f), 20.0f, r, g, b, 0, true , 0.01);
}



bool FileExists(const char *fname){
	return access(fname, 0) != -1;
}

inline edict_t *PEntityOfEntIndex(int iEntIndex)
{
	if (iEntIndex >= 0 && iEntIndex < globals->maxEntities)
	{
		return (edict_t *)(globals->pEdicts + iEntIndex);
	}
	return NULL;
}

inline edict_t *Player(){
	return PEntityOfEntIndex(1);
}

inline bool SP(){
	if(globals->pEdicts!=NULL){
		return true;
	}
	else{
		return false;
	}
}

con_nprint_t HudMsg(float R1=0.0, float G1=0.5, float B1=0.9, float T1=5.0){

	con_nprint_t hud_msg;
	hud_msg.fixed_width_font = true;
	hud_msg.color[0] = R1;
	hud_msg.color[1] = G1;
	hud_msg.color[2] = B1;
	hud_msg.index = 0;
	hud_msg.time_to_live = T1;
	
	return hud_msg;
}

void RecordDemo(){
	std::string CurrentMapName = "null";
	if(strlen(ienginetool->GetCurrentMap())>0){
		CurrentMapName = std::string(ienginetool->GetCurrentMap());
		CurrentMapName.replace(CurrentMapName.find("maps/"),5,"");
		CurrentMapName.replace(CurrentMapName.find(".bsp"),4,"");
		}
	char* record = "record ";
	char buffer[80];
	time_t seconds = time(NULL);
	tm* timeinfo = localtime(&seconds);
	char* format = "%Y_%m_%d_%H-%M-%S";
	strftime(buffer, 80, format, timeinfo);
	
	std::string str_record = std::string(record);
	std::string str_buffer = std::string(buffer);
	std::string demoname = str_record + CurrentMapName + "_" + str_buffer + "\n";
	std::string hud_info = "Start recording to " + CurrentMapName + "_" + str_buffer + ".dem\n";

	ConVar * drc = NULL;
	drc = g_pCVar->FindVar("demo_recordcommands");
	if(drc == NULL){
		Warning("Failed to find ConVar demo_recordcommands!\n");
		return;
		}
	drc->RemoveFlags(drc->GetFlags());

	engine_sv->ServerCommand("demo_recordcommands 0\n");
	engine_sv->ServerCommand(demoname.c_str());
	
	engine_sv->Con_NXPrintf(&HudMsg(R,G,B,T),hud_info.c_str());
	ConColorMsg(Color(0,128,255,255),hud_info.c_str());
	engine_sv->ServerCommand("playgamesound UI\\BeepClear.wav\n");
}

void PluginUnload(){
	isThreadActive = false;
	L4DAimBotPZ.SetValue(0);
	L4DAimBotInfected.SetValue(0);
	L4DEspPZ.SetValue(0);
	L4DEspInfected.SetValue(0);
	L4DHunterDistanceMeter.SetValue(0);
	DrawPZGlow.SetValue(0);
	DrawViewLine.SetValue(0);
	KillCounter.SetValue(0);
	L4DBhop.SetValue(0);
	ShowItems.SetValue(0);
	ShowPlayersHealth.SetValue(0);
	TankHelpers.SetValue(0);
	L4DTest2.SetValue(0);
	engine_sv->ServerCommand("plugin_unload L4D_Versus_Tool\n");
	ConVar_Unregister( );
	DisconnectTier2Libraries( );
	DisconnectTier1Libraries( );
}

/*char *GetPlayerNameByID(int id){
	int ents = entList->GetMaxEntities();
	IClientEntity *cl_ent;
	C_BaseEntity *pEntity = NULL;
	player_info_t info;
	
	for (int i=0; i<ents; i++){
		cl_ent = entList->GetClientEntity(i);
		if(cl_ent!=null){
			if(strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"CTerrorPlayer")==0){
				engine_sv->GetPlayerInfo(i,&info);
				if(id==info.userID){
				break;
				}
			}
		}
	}
	return info.name;
}*/

void ZombieCounter(bool zombie=false, bool headshot=false, bool player=false){
	//Count
	if(zombie){
		zombies++;
	}
	if(headshot){
		headshots++;
	}
	if(player){
		players++;
	}
	//Show msg
	if(zombie){
		engine_sv->Con_NXPrintf(&HudMsg(0.9,0.9,0.9,9.0),"Killed zombies: %d",zombies);
	}
	if(zombie&&headshot){
		engine_sv->ServerCommand("playgamesound UI\\LittleReward.wav\n");
		engine_sv->Con_NXPrintf(&HudMsg(0.9,0.9,0.9,9.0),"Killed zombies: %d (Headshot: %d)",zombies,headshots);
	}
	if(player){
		engine_sv->Con_NXPrintf(&HudMsg(0.9,0.9,0.9,9.0),"Killed players: %d",players);
	}
}

void Draw_PZTank_HealthBar(int perc ,int ent, int pos, int dur, int r, int g, int b, int alp){
	if(perc==100||perc>95&&perc<100) vphysics->AddEntityTextOverlay(ent,pos,dur,r,g,b,alp,"(<----------|---------->)");
	if(perc==95||perc>90&&perc<95) vphysics->AddEntityTextOverlay(ent,pos,dur,r,g,b,alp,"(<----------|--------->)");
	if(perc==90||perc>85&&perc<90) vphysics->AddEntityTextOverlay(ent,pos,dur,r,g,b,alp,"(<----------|-------->)");
	if(perc==85||perc>80&&perc<85) vphysics->AddEntityTextOverlay(ent,pos,dur,r,g,b,alp,"(<----------|------->)");
	if(perc==80||perc>75&&perc<80) vphysics->AddEntityTextOverlay(ent,pos,dur,r,g,b,alp,"(<----------|------>)");
	if(perc==75||perc>70&&perc<75) vphysics->AddEntityTextOverlay(ent,pos,dur,r,g,b,alp,"(<----------|----->)");
	if(perc==70||perc>65&&perc<70) vphysics->AddEntityTextOverlay(ent,pos,dur,r,g,b,alp,"(<----------|---->)");
	if(perc==65||perc>60&&perc<65) vphysics->AddEntityTextOverlay(ent,pos,dur,r,g,b,alp,"(<----------|--->)");
	if(perc==60||perc>55&&perc<60) vphysics->AddEntityTextOverlay(ent,pos,dur,r,g,b,alp,"(<----------|-->)");
	if(perc==55||perc>50&&perc<55) vphysics->AddEntityTextOverlay(ent,pos,dur,r,g,b,alp,"(<----------|->)");
	if(perc==50||perc>45&&perc<50) vphysics->AddEntityTextOverlay(ent,pos,dur,r,g,b,alp,"(<----------|>)");
	if(perc==45||perc>40&&perc<45) vphysics->AddEntityTextOverlay(ent,pos,dur,r,g,b,alp,"(<---------|>)");
	if(perc==40||perc>35&&perc<40) vphysics->AddEntityTextOverlay(ent,pos,dur,r,g,b,alp,"(<--------|>)");
	if(perc==35||perc>30&&perc<35) vphysics->AddEntityTextOverlay(ent,pos,dur,r,g,b,alp,"(<-------|>)");
	if(perc==30||perc>25&&perc<30) vphysics->AddEntityTextOverlay(ent,pos,dur,r,g,b,alp,"(<------|>)");
	if(perc==25||perc>20&&perc<25) vphysics->AddEntityTextOverlay(ent,pos,dur,r,g,b,alp,"(<-----|>)");
	if(perc==20||perc>15&&perc<20) vphysics->AddEntityTextOverlay(ent,pos,dur,r,g,b,alp,"(<----|>)");
	if(perc==15||perc>10&&perc<15) vphysics->AddEntityTextOverlay(ent,pos,dur,r,g,b,alp,"(<---|>)");
	if(perc==10||perc>5&&perc<10) vphysics->AddEntityTextOverlay(ent,pos,dur,r,g,b,alp,"(<--|>)");
	if(perc==5) vphysics->AddEntityTextOverlay(ent,pos,dur,r,g,b,alp,"(<-|>)");
	if(perc<5) vphysics->AddEntityTextOverlay(ent,pos,dur,r,g,b,alp,"(<0>)");
}

bool isOwnner(int num){
	IClientEntity* pLocalPlayer = entList->GetClientEntity( client->GetLocalPlayer());
	if(pLocalPlayer->entindex()==num) return true;
	else return false;
}

CBaseEntity *GetOwnerEntity(){
	IClientEntity* OwnerPlayer = entList->GetClientEntity( client->GetLocalPlayer());
	return OwnerPlayer->GetBaseEntity();
}

int GetOwnerTeam(){
	CBaseEntity *owner = GetOwnerEntity();
	return  *(int*) ( ( DWORD )owner + ( DWORD )0x0BC );
}

Vector GetOwnerPos(){
	CBaseEntity *Owner = GetOwnerEntity();
	float *own_x = (float*) ( ( DWORD )Owner + ( DWORD )0x00FC );
	float *own_y = (float*) ( ( DWORD )Owner + ( DWORD )0x0100 );
	float *own_z = (float*) ( ( DWORD )Owner + ( DWORD )0x0104 );
	Vector OwnerPos(*own_x,*own_y,*own_z);
	return OwnerPos;
}

float GetOwnerVel(){
	Vector m_vecVelocity = *(Vector *)((char *)GetOwnerEntity() + 96);
	return m_vecVelocity.Length();
}

void GetOwnerPos2(Vector *OwnerPos){
	CBaseEntity *Owner = GetOwnerEntity();
	float *own_x = (float*) ( ( DWORD )Owner + ( DWORD )0x00FC );
	float *own_y = (float*) ( ( DWORD )Owner + ( DWORD )0x0100 );
	float *own_z = (float*) ( ( DWORD )Owner + ( DWORD )0x0104 );
	*OwnerPos = Vector(*own_x,*own_y,*own_z);
}

inline C_BasePlayer *EntityToBasePlayer( C_BaseEntity *pEntity ){
	return static_cast<C_BasePlayer *>( pEntity );
}

CBasePlayer *GetOwnerPlayer(){
	CBasePlayer *player = EntityToBasePlayer(GetOwnerEntity());
	return player;
}

void GetBonePosition ( CBaseEntity* pEntity, Vector *BonePos, int Bone_num ){
	/*Reverse from client.dll C_BaseAnimating::SetupBone
	10EBF663    0BC1            OR EAX,ECX
	10EBF665    894D 10         MOV DWORD PTR SS:[EBP+10],ECX
	10EBF668    8986 64060000   MOV DWORD PTR DS:[ESI+664],EAX
	10EBF66E    8986 60060000   MOV DWORD PTR DS:[ESI+660],EAX*/
	
	DWORD BoneBase = *(DWORD*)((DWORD)pEntity + 0x660);
	float bone_pos[3] = {0,0,0};
	bone_pos[0] = *(float*)((DWORD)BoneBase + 0x30 * Bone_num + 0x0C);
	bone_pos[1] = *(float*)((DWORD)BoneBase + 0x30 * Bone_num + 0x1C);
	bone_pos[2] = *(float*)((DWORD)BoneBase + 0x30 * Bone_num + 0x2C);
	*BonePos = Vector(bone_pos[0],bone_pos[1],bone_pos[2]);
}

Vector GetBoneOrign ( CBaseEntity* pEntity,  int Bone_num ){
	/*Reverse from client.dll C_BaseAnimating::SetupBone
	10EBF663    0BC1            OR EAX,ECX
	10EBF665    894D 10         MOV DWORD PTR SS:[EBP+10],ECX
	10EBF668    8986 64060000   MOV DWORD PTR DS:[ESI+664],EAX
	10EBF66E    8986 60060000   MOV DWORD PTR DS:[ESI+660],EAX*/
	if(!pEntity)
		return Vector(0,0,0);
	DWORD BoneBase = *(DWORD*)((DWORD)pEntity + 0x660);
	if(!BoneBase)
		return Vector(0,0,0);
	float bone_pos[3] = {0,0,0};
	bone_pos[0] = *(float*)((DWORD)BoneBase + 0x30 * Bone_num + 0x0C);
	bone_pos[1] = *(float*)((DWORD)BoneBase + 0x30 * Bone_num + 0x1C);
	bone_pos[2] = *(float*)((DWORD)BoneBase + 0x30 * Bone_num + 0x2C);
	return Vector(bone_pos[0],bone_pos[1],bone_pos[2]);
}

bool GetBoneOrignP ( CBaseEntity* pEntity, Vector *orign, int Bone_num ){
	/*Reverse from client.dll C_BaseAnimating::SetupBone
	10EBF663    0BC1            OR EAX,ECX
	10EBF665    894D 10         MOV DWORD PTR SS:[EBP+10],ECX
	10EBF668    8986 64060000   MOV DWORD PTR DS:[ESI+664],EAX
	10EBF66E    8986 60060000   MOV DWORD PTR DS:[ESI+660],EAX*/
	if(!pEntity)
		return false;
	DWORD BoneBase = *(DWORD*)((DWORD)pEntity + 0x660);
	if(!BoneBase)
		return false;
	float bone_pos[3] = {0,0,0};
	bone_pos[0] = *(float*)((DWORD)BoneBase + 0x30 * Bone_num + 0x0C);
	bone_pos[1] = *(float*)((DWORD)BoneBase + 0x30 * Bone_num + 0x1C);
	bone_pos[2] = *(float*)((DWORD)BoneBase + 0x30 * Bone_num + 0x2C);
	if(Vector(bone_pos[0],bone_pos[1],bone_pos[2]).Length()==0)
		return false;
	*orign = Vector(bone_pos[0],bone_pos[1],bone_pos[2]);
	return true;
}

inline Vector GetBonePos(int entity, int bone){
	IClientEntity *cl_ent;
	C_BaseEntity *pEntity = NULL;
	cl_ent = entList->GetClientEntity(entity);
	IClientRenderable* RenderEntity = cl_ent->GetClientRenderable();

	matrix3x4_t bone_matrix[MAXSTUDIOBONES];
	RenderEntity->SetupBones( bone_matrix, 128, BONE_USED_BY_HITBOX, 0);
	Vector pos;
	QAngle ang;
	pos.Init();
	ang.Init();
	MatrixAngles( bone_matrix[ bone ], ang, pos );
	return pos;
}

const VMatrix GetWorldToScreenMatrix(){
	return client->WorldToScreenMatrix();
}

bool BoneBone(IClientEntity *ent, Vector *pos, QAngle *ang, int bone){
	IClientRenderable* RenderEntity = ent->GetClientRenderable();
	matrix3x4_t bone_matrix2[MAXSTUDIOBONES];
	if(RenderEntity->SetupBones( bone_matrix2, 128, BONE_USED_BY_HITBOX, 0)){
		matrix3x4_t HitboxMatrix = bone_matrix2[bone];
		*pos = Vector(HitboxMatrix[0][3], HitboxMatrix[1][3], HitboxMatrix[2][3]);
		return true;
	}
	return false;
}

QAngle GetBoneAng(int entity, int bone){
	IClientEntity *cl_ent;
	C_BaseEntity *pEntity = NULL;
	cl_ent = entList->GetClientEntity(entity);
	IClientRenderable* RenderEntity = cl_ent->GetClientRenderable();

	matrix3x4_t bone_matrix[MAXSTUDIOBONES];
	RenderEntity->SetupBones( bone_matrix, 128, BONE_USED_BY_HITBOX, 0);
	Vector pos;
	QAngle ang;
	pos.Init();
	ang.Init();
	MatrixAngles( bone_matrix[ bone ], ang, pos );
	return ang;
}

const model_t *GetEntityModel(CBaseEntity* pEntity){
	ICollideable *pCollideable = pEntity->GetCollideable();
	return pCollideable->GetCollisionModel();
}

int GetOwnerAimEnt(){
	CBaseEntity *owner = NULL;
	Vector eye_position, aim_dir, vec_end;
	QAngle eye_angles;

	eye_position.Init();
	aim_dir.Init();
	vec_end.Init();
	eye_angles.Init();

	owner = GetOwnerEntity();
	eye_position = GetOwnerPos();
	engine_cl->GetViewAngles(eye_angles);
	AngleVectors(eye_angles, &aim_dir);
	VectorNormalize(aim_dir);
	vec_end = eye_position + aim_dir * 8000;
	debug->AddLineOverlay(GetOwnerPos(),vec_end,255,255,255,true,0);

	Ray_t  ray;
	ray.Init(GetOwnerPos()+Vector(0,0,75),vec_end);
	trace_t tr;

	enginetrace->TraceRay(ray,MASK_NPCWORLDSTATIC|MASK_VISIBLE_AND_NPCS|MASK_SHOT,NULL,&tr);

	if(!tr.m_pEnt) return 0;
	return tr.m_pEnt->entindex();
}

Vector GetEntityOrign(CBaseEntity *entity){	
	if(!entity) return Vector(0,0,0);
	Vector m_vecOrigin = *(Vector*) ( ( DWORD )entity + 0xFC ); 
	return m_vecOrigin;
}

bool GetEntityOrignP(CBaseEntity *entity, Vector *orign){	
	if(!entity)
		return false;
	Vector m_vecOrigin;
	m_vecOrigin.Init();
	m_vecOrigin	= *(Vector*) ( ( DWORD )entity + 0xFC ); 
	if(m_vecOrigin.Length()==0)
		return false;
	*orign = *(Vector*) ( ( DWORD )entity + 0xFC );
	return true;
}

QAngle GetEntityRotation(CBaseEntity *entity){	
	if(!entity) return QAngle(0,0,0);
	QAngle m_angRotation = *(QAngle*) ( ( DWORD )entity + 0xF0 ); 
	return m_angRotation;
}

QAngle GetPlayerEyeAngles(){
	CBaseEntity *Player  = entList->GetClientEntity(client->GetLocalPlayer())->GetBaseEntity();
	float *view_x = (float*) ( ( DWORD )Player + ( DWORD )0x16F4 );
	float *view_y = (float*) ( ( DWORD )Player + ( DWORD )0x16F8 );
	//float *view_z = (float*) ( ( DWORD )Player + ( DWORD )0xD4 );
	QAngle playerView(*view_x,*view_y,0);
	return playerView;
}

void DrawEyeViewLine(CBaseEntity *pEntity){
	Vector ent_orign, aim_dir;
	QAngle eye_angles;

	ent_orign = GetEntityOrign(pEntity);

	float *view_x = (float*) ( ( DWORD )pEntity + ( DWORD )0x16F4 );
	float *view_y = (float*) ( ( DWORD )pEntity + ( DWORD )0x16F8 );
	//float *view_z = (float*) ( ( DWORD )Player + ( DWORD )0xD4 );
	eye_angles =  QAngle(*view_x,*view_y,0);

	AngleVectors(eye_angles, &aim_dir);
	VectorNormalize(aim_dir);
	Vector vec_end = ent_orign + aim_dir * L4DTest3.GetFloat();//8000
	debug->AddLineOverlay(GetBoneOrign(pEntity,14),vec_end,255,255,255,true,0);
	debug->AddBoxOverlay(vec_end,Vector(-2,-2,-2),Vector(2,2,2),eye_angles,0,255,0,255,0);
}

IClientMode *GetClientModeInterface(){
	IClientMode *clmod = NULL;
	typedef IClientMode *(*FuncType)();
	FuncType ClMode;

	ClMode = (FuncType)FindPattern("client.dll",GetClientMode_SIG,GetClientMode_MASK);
	if(ClMode)
		clmod = ClMode();
	else
		clmod = NULL;
	return clmod;
}

IViewRender *GetIViewRenderInterface(){
	IViewRender *view_render = NULL;
	typedef IViewRender *(*FuncType)();
	FuncType GetViewRenderInstance;

	GetViewRenderInstance = (FuncType)FindPattern("client.dll",GetViewRenderInstance_SIG,GetViewRenderInstance_MASK);
	if(GetViewRenderInstance)
		view_render = GetViewRenderInstance();
	else
		view_render = NULL;
	return view_render;
}

void Circle( const Vector &position, const Vector &xAxis, const Vector &yAxis, float radius, int r, int g, int b, int a, bool bNoDepthTest, float flDuration )
{
	const int nSegments = 16;
	const float flRadStep = (M_PI*2.0f) / (float) nSegments;

	Vector vecLastPosition;
	
	// Find our first position
	// Retained for triangle fanning
	Vector vecStart = position + xAxis * radius;
	Vector vecPosition = vecStart;

	// Draw out each segment (fanning triangles if we have an alpha amount)
	for ( int i = 1; i <= nSegments; i++ )
	{
		// Store off our last position
		vecLastPosition = vecPosition;

		// Calculate the new one
		float flSin, flCos;
		SinCos( flRadStep*i, &flSin, &flCos );
		vecPosition = position + (xAxis * flCos * radius) + (yAxis * flSin * radius);

		// Draw the line
		debug->AddLineOverlay( vecLastPosition, vecPosition, r, g, b, bNoDepthTest, flDuration );

		// If we have an alpha value, then draw the fan
		if ( a && i > 1 )
		{		
			debug->AddTriangleOverlay( vecStart, vecLastPosition, vecPosition, r, g, b, a, bNoDepthTest, flDuration );
		}
	}
}

void DrawCircle( const Vector &position, const QAngle &angles, float radius, int r, int g, int b, int a, bool bNoDepthTest, float flDuration )
{
	// Setup our transform matrix
	matrix3x4_t xform;
	AngleMatrix( angles, position, xform );
	Vector xAxis, yAxis;
	// default draws circle in the y/z plane
	MatrixGetColumn( xform, 2, xAxis );
	MatrixGetColumn( xform, 1, yAxis );
	Circle( position, xAxis, yAxis, radius, r, g, b, a, bNoDepthTest, flDuration );
}

void DrawAimPoint(CBaseEntity *entity, Vector &position){
	AimPointTick++;
	if(AimPointTick==359) AimPointTick = 0;
	debug->AddBoxOverlay(position,Vector(-2,-2,-2),Vector(2,2,2),QAngle(AimPointTick,0,0),255,0,0,5,0);
}

void Sphere( const Vector &position, const QAngle &angles, float radius, int r, int g, int b, int a, bool bNoDepthTest, float flDuration )
{
	// Setup our transform matrix
	matrix3x4_t xform;
	AngleMatrix( angles, position, xform );
	Vector xAxis, yAxis, zAxis;
	// default draws circle in the y/z plane
	MatrixGetColumn( xform, 0, xAxis );
	MatrixGetColumn( xform, 1, yAxis );
	MatrixGetColumn( xform, 2, zAxis );
	Circle( position, xAxis, yAxis, radius, r, g, b, a, bNoDepthTest, flDuration );	// xy plane
	Circle( position, yAxis, zAxis, radius, r, g, b, a, bNoDepthTest, flDuration );	// yz plane
	Circle( position, xAxis, zAxis, radius, r, g, b, a, bNoDepthTest, flDuration );	// xz plane
}

void DrawESPBox(Vector &position, const Vector &mins, const Vector &maxs, int r, int g, int b){
	if(position.Length()!=0){
		//debug->AddBoxOverlay(EntOrign,Vector(-2,-2,-2),Vector(2,2,2),QAngle(0,0,0),r,g,b,255,0);
		debug->AddLineOverlay(position+Vector(mins.x,maxs.y,0),position+Vector(maxs.x,maxs.y,0),r,g,b,true,0); //Side 1 - bottom forward
		debug->AddLineOverlay(position+Vector(mins.x,mins.y,0),position+Vector(maxs.x,mins.y,0),r,g,b,true,0); //Side 2 - bottom backward
		debug->AddLineOverlay(position+Vector(mins.x,mins.y,0),position+Vector(mins.x,maxs.y,0),r,g,b,true,0); //Side 3 - bottom left
		debug->AddLineOverlay(position+Vector(maxs.x,mins.y,0),position+Vector(maxs.x,maxs.y,0),r,g,b,true,0); //Side 4 - bottom right

		debug->AddLineOverlay(position+Vector(mins.x,maxs.y,maxs.z),position+Vector(maxs.x,maxs.y,maxs.z),r,g,b,true,0); //Side 5 - top forward
		debug->AddLineOverlay(position+Vector(mins.x,mins.y,maxs.z),position+Vector(maxs.x,mins.y,maxs.z),r,g,b,true,0); //Side 6 - top backward
		debug->AddLineOverlay(position+Vector(mins.x,mins.y,maxs.z),position+Vector(mins.x,maxs.y,maxs.z),r,g,b,true,0); //Side 7 - top left
		debug->AddLineOverlay(position+Vector(maxs.x,mins.y,maxs.z),position+Vector(maxs.x,maxs.y,maxs.z),r,g,b,true,0); //Side 8 - top right

		debug->AddLineOverlay(position+Vector(mins.x,maxs.y,0),position+Vector(mins.x,maxs.y,maxs.z),r,g,b,true,0); //Side 9
		debug->AddLineOverlay(position+Vector(maxs.x,maxs.y,0),position+Vector(maxs.x,maxs.y,maxs.z),r,g,b,true,0); //Side 10
		debug->AddLineOverlay(position+Vector(mins.x,mins.y,0),position+Vector(mins.x,mins.y,maxs.z),r,g,b,true,0); //Side 11
		debug->AddLineOverlay(position+Vector(maxs.x,mins.y,0),position+Vector(maxs.x,mins.y,maxs.z),r,g,b,true,0); //Side 12*/
	}
}

//Set of initial variables you'll need
//Our desktop handle
RECT m_Rect; 
HDC HDC_Desktop;
//Brush to paint ESP etc
HBRUSH EnemyBrush;
HFONT Font; //font we use to write text with
//ESP VARS
const DWORD dw_vMatrix = 0x58C45C;
const DWORD dw_antiFlick = 0x58C2B8;
HWND TargetWnd;
HWND Handle;
DWORD DwProcId;
COLORREF SnapLineCOLOR;
COLORREF TextCOLOR; 

void SetupDrawing(HDC hDesktop, HWND handle)
{
	HDC_Desktop = hDesktop;
	Handle = handle;
	EnemyBrush = CreateSolidBrush(RGB(255, 216, 0));
	//Color
	SnapLineCOLOR = RGB(0, 0, 255);
	TextCOLOR = RGB(0, 255, 0);
}

void DrawLine(float StartX, float StartY, float EndX, float EndY, COLORREF Pen)
{
	int a,b=0;
	HPEN hOPen;
	// penstyle, width, color
	HPEN hNPen = CreatePen(PS_SOLID, 2, Pen);
	hOPen = (HPEN)SelectObject(HDC_Desktop, hNPen);
	// starting point of line
	MoveToEx(HDC_Desktop, StartX, StartY, NULL);
	// ending point of line
	a = LineTo(HDC_Desktop, EndX, EndY);
	DeleteObject(SelectObject(HDC_Desktop, hOPen));
}

void DrawFilledRect(int x, int y, int w, int h)
{
	//We create our rectangle to draw on screen
	RECT rect = { x, y, x + w, y + h }; 
	//We clear that portion of the screen and display our rectangle
	FillRect(HDC_Desktop, &rect, EnemyBrush);
}

void DrawBorderBox(int x, int y, int w, int h, int thickness)
{
	//Top horiz line
	DrawFilledRect(x, y, w, thickness);
	//Left vertical line
	DrawFilledRect( x, y, thickness, h);
	//right vertical line
	DrawFilledRect((x + w), y, thickness, h);
	//bottom horiz line
	DrawFilledRect(x, y + h, w+thickness, thickness);
}

void DrawString(int x, int y, COLORREF color, const char* text)
{	
	SetTextAlign(HDC_Desktop,TA_CENTER|TA_NOUPDATECP);

	SetBkColor(HDC_Desktop,RGB(0,0,0));
	SetBkMode(HDC_Desktop,TRANSPARENT);

	SetTextColor(HDC_Desktop,color);

	SelectObject(HDC_Desktop,Font);

	TextOutA(HDC_Desktop,x,y,text,strlen(text));

	DeleteObject(Font);
}


bool WorldToScreen( const Vector& point, Vector& screen )
{
	float w = 0.0f;
	VMatrix& worldToScreen = *(VMatrix*)(( DWORD )GetModuleHandleA("engine.dll")+( DWORD )0x520A94);

	screen.x = worldToScreen[0][0] * point[0] + worldToScreen[0][1] * point[1] + worldToScreen[0][2] * point[2] + worldToScreen[0][3];
	screen.y = worldToScreen[1][0] * point[0] + worldToScreen[1][1] * point[1] + worldToScreen[1][2] * point[2] + worldToScreen[1][3];
	w		 = worldToScreen[3][0] * point[0] + worldToScreen[3][1] * point[1] + worldToScreen[3][2] * point[2] + worldToScreen[3][3];

	if(w < 0.01f)
		return false;

	float invw = 1.0f / w;
	screen.x *= invw;
	screen.y *= invw;

	int width = (int)(m_Rect.right - m_Rect.left);
	int height = (int)(m_Rect.bottom - m_Rect.top);

	float x = width/2;
	float y = height/2;

	x += 0.5 * screen.x * width + 0.5;
	y -= 0.5 * screen.y * height + 0.5;

	screen.x = x+ m_Rect.left;
	screen.y = y+ m_Rect.top ;

	return true;
}


/*void FindPoint(Vector &point, int screenwidth, int screenheight, int degrees)
{
	float x2 = screenwidth / 2;
	float y2 = screenheight / 2;

	float d = sqrt(pow((point.x - x2),2) + (pow((point.y - y2),2))); //Distance
	float r = degrees / d; //Segment ratio
	
	point.x = r * point.x + (1 - r) * x2; //find point that divides the segment
	point.y = r * point.y + (1 - r) * y2; //into the ratio (1-r):r
}

 bool ScreenTransform( const Vector &point, Vector &screen )
 {
	float w;
	VMatrix& worldToScreen = *(VMatrix*)(( DWORD )GetModuleHandleA("engine.dll")+( DWORD )0x521D10);
	screen.x = worldToScreen[0][0] * point[0] + worldToScreen[0][1] * point[1] + worldToScreen[0][2] * point[2] + worldToScreen[0][3];
	screen.y = worldToScreen[1][0] * point[0] + worldToScreen[1][1] * point[1] + worldToScreen[1][2] * point[2] + worldToScreen[1][3];
	w		 = worldToScreen[3][0] * point[0] + worldToScreen[3][1] * point[1] + worldToScreen[3][2] * point[2] + worldToScreen[3][3];
	screen.z = 0.0f;
	bool behind = false;
	if( w < 0.001f ){
		behind = true;
		float invw = -1.0f / w;
		screen.x *= invw;
		screen.y *= invw;
	}
	else{
		behind = false;
		float invw = 1.0f / w;
		screen.x *= invw;
		screen.y *= invw;
	}
	return behind;
}

  bool WorldToScreen( const Vector &vOrigin, Vector &vScreen )
 {
	bool st = ScreenTransform(vOrigin, vScreen);
	int iScreenWidth, iScreenHeight;
	//engine_cl->GetScreenSize( iScreenWidth, iScreenHeight );
	iScreenWidth = 600;
	iScreenHeight = 800;
	float x = iScreenWidth / 2;
	float y = iScreenHeight / 2;
	x += 0.5 * vScreen.x * iScreenWidth + 0.5;
	y -= 0.5 * vScreen.y * iScreenHeight + 0.5;
	vScreen.x = x;
	vScreen.y = y;
	if(vScreen.x > iScreenHeight || vScreen.x < 0 || vScreen.y > iScreenWidth || vScreen.y < 0 || st)
	{
		FindPoint(vScreen, iScreenWidth, iScreenHeight, iScreenHeight/2);
		return false;
	}
	return true;
}*/

void Draw_Skeleton_Infected(){
	int max_entitys = entList->GetMaxEntities();
	IClientEntity *cl_ent;
	studiohdr_t *mob_hdr = NULL;
	mstudiobone_t *mob_bone = NULL;
	matrix3x4_t bone_matrix[MAXSTUDIOBONES];
	ICollideable *pCollideable = NULL;
	vcollide_t *pCollide = NULL;
	int pColideFlags;
	Vector vec_from,vec_to;
	QAngle ang_from,ang_to;

	for(int i=0; i<max_entitys; i++){	
		cl_ent = entList->GetClientEntity(i);
		if (cl_ent==NULL||cl_ent->IsDormant()) continue;
		if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Infected")){
			mob_hdr = model_info->GetStudiomodel(GetEntityModel(cl_ent->GetBaseEntity()));
			if(!mob_hdr) continue;
			pCollideable = cl_ent->GetCollideable();
			if(!pCollideable) continue;
			pCollide = model_info->GetVCollide(pCollideable->GetCollisionModel());
			if(!pCollide) continue;
			pColideFlags = pCollideable->GetSolidFlags();
			if(pColideFlags!=20){
				IClientRenderable* RenderEntity = cl_ent->GetClientRenderable();
				if(!RenderEntity) continue;
				if(RenderEntity->SetupBones( bone_matrix, 128, BONE_USED_BY_HITBOX, client->Time())){
					for(int i=0;i<mob_hdr->numbones;i++){
						mob_bone = mob_hdr->pBone(i);
						if(!mob_bone) continue;
						if (!(mob_bone->flags & BONE_USED_BY_HITBOX)) continue;
						debug->AddCoordFrameOverlay(bone_matrix[i],3.0f);
						int iParentIndex;
						iParentIndex = mob_bone->parent;
						if (iParentIndex<0) continue;
						MatrixAngles(bone_matrix[i],ang_from,vec_from);
						MatrixAngles(bone_matrix[iParentIndex],ang_to,vec_to);
						debug->AddLineOverlay(vec_from,vec_to,0,255,255,true,0.0f);
						debug->ClearDeadOverlays();
					}
				}
			}
		}
	}
}


DWORD WINAPI L4D_VT_MainThread (LPVOID){

		for(;;Sleep(ThreadRate.GetInt())){
			if(!isThreadActive){
				WaitForSingleObject(l4d_thread, INFINITE);
				CloseHandle(l4d_thread);
			}

			//ThreadDebug
			if(ThreadDebug.GetBool()){
				ConColorMsg(Color(255,216,0,255),"RandInt: ");
				ConColorMsg(Color(0,255,0,255),"%d\n",rand()%ThreadDebugRange.GetInt()+1);
			}

			//Bunny Hop
			if(L4DBhop.GetBool()&&client->IsInGame()&&GetAsyncKeyState(SPACE_BAR)){
				if(GetAsyncKeyState(0x11)) engine_sv->ServerCommand("-jump\n");
				CBaseEntity *Owner = GetOwnerEntity();
				int *m_flags = (int*) ( ( DWORD )Owner + ( DWORD )0xC8 );

				if(*m_flags==129||*m_flags==641){
					engine_sv->ServerCommand("+jump\n");
				}
				else if(*m_flags==130){
					engine_sv->ServerCommand("-jump\n");
				}	
			}

			//Esp PZ
			if(L4DEspPZ.GetBool()&&client->IsInGame()){
				int PZ_BotClass;
				int PZ_BotHealth;
				int max_entitys = entList->GetMaxEntities();
				IClientEntity *cl_ent;
				C_BaseEntity *PZ = NULL;
				
				for(int i=0; i<max_entitys; i++){
					cl_ent = entList->GetClientEntity(i);
					if(cl_ent!=null){
						if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Smoker")||
						!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Boomer")||
						!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Hunter")||
						!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Tank")){
							PZ = cl_ent->GetBaseEntity();
							int *PZ_BotHealth = (int*) ( ( DWORD )PZ + ( DWORD )0xC4 );
							PZ_BotClass = *(int *)((char *)PZ + 6500);

							switch(PZ_BotClass){
								case ZC_SMOKER:
									{
										if(*PZ_BotHealth>2){
											debug->AddLineOverlay(GetOwnerPos(),GetEntityOrign(PZ),255,255,0,true,0);debug->AddBoxOverlay(GetEntityOrign(PZ),Vector(-20,-20,0),Vector(20,20,85),QAngle(0,0,0),255,255,0,L4DEspPZAlpha.GetInt(),0);break;
										}
									}
								case ZC_BOOMER:
									{
										if(*PZ_BotHealth>2){
											debug->AddLineOverlay(GetOwnerPos(),GetEntityOrign(PZ),255,170,60,true,0);debug->AddBoxOverlay(GetEntityOrign(PZ),Vector(-20,-20,0),Vector(20,20,70),QAngle(0,0,0),255,170,60,5,0);break;
										}
									}
								case ZC_HUNTER:
									{
										if(*PZ_BotHealth>2){
											debug->AddLineOverlay(GetOwnerPos(),GetEntityOrign(PZ),0,150,255,true,0);debug->AddBoxOverlay(GetEntityOrign(PZ),Vector(-20,-20,0),Vector(20,20,65),QAngle(0,0,0),0,150,255,5,0);break;
										}
									}
								case ZC_TANK:
									{
										if(*PZ_BotHealth>2){
											debug->AddLineOverlay(GetOwnerPos(),GetEntityOrign(PZ),0,255,0,true,0);debug->AddBoxOverlay(GetEntityOrign(PZ),Vector(-20,-20,0),Vector(20,20,75),QAngle(0,0,0),0,255,0,5,0);break;
										} 
									}
								default:
									{
									}
							}
						}
					}
				}
				//Esp Witch
				C_BaseEntity *Witch = NULL;
				for(int i=0; i<max_entitys; i++){
					cl_ent = entList->GetClientEntity(i);
					if(cl_ent!=null){
						if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Witch")){
							Witch = cl_ent->GetBaseEntity();
							debug->AddLineOverlay(GetOwnerPos(),GetEntityOrign(Witch),255,0,220,true,0);
							Vector Orign,Head;
							Orign.Init();
							Head.Init();
							vec_t BoxZ;
							
							if(GetEntityOrignP(Witch,&Orign)&&GetBoneOrignP(Witch,&Head,14)){
								BoxZ = (Head.z - Orign.z) + 10;
								debug->AddBoxOverlay(Orign,Vector(-15,-15,0),Vector(15,15,BoxZ),QAngle(0,0,0),255,0,220,5,0);
							}
						}
					}
				}
				//Esp Player PZ
				C_BaseEntity *PZ_Player = NULL;
				int PZ_PlayerClass;
				int *PZ_PlayerHealth;
				int PZ_InitialHealth;
				bool isGhost = false;

				for(int i=0; i<max_entitys; i++){
					if(isOwnner(i)) continue;
					cl_ent = entList->GetClientEntity(i);
					if(cl_ent!=null){
						if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"CTerrorPlayer")){
							PZ_Player = cl_ent->GetBaseEntity();
							int *PlayerTeam = (int*) ( ( DWORD )PZ_Player + ( DWORD )0x0BC );
							if(*PlayerTeam==Infected){
								PZ_PlayerClass = *(int*)((char *)PZ_Player + 6500);
								PZ_PlayerHealth = (int*) ( ( DWORD )PZ_Player + ( DWORD )0xC4 );
								PZ_InitialHealth = *(int *)((char *)PZ_Player + 7336);
								isGhost = *(bool *)((char *)PZ_Player + 6511);
								Vector Orign,Head;
								Orign.Init();
								Head.Init();
								vec_t BoxZ;
								if(GetEntityOrignP(PZ_Player,&Orign)&&GetBoneOrignP(PZ_Player,&Head,14)){
									BoxZ = (Head.z - Orign.z) + 10;
									switch(PZ_PlayerClass){
										case ZC_SMOKER:
										{
											if(*PZ_PlayerHealth>2){
												if(isGhost){
													debug->AddBoxOverlay(Orign,Vector(-20,-20,0),Vector(20,20,75),QAngle(0,0,0),255,255,0,L4DEspPZAlpha.GetInt(),0);break;
												}
												else{
													debug->AddLineOverlay(GetOwnerPos(),Orign,255,255,0,true,0);debug->AddBoxOverlay(Orign,Vector(-20,-20,0),Vector(20,20,BoxZ),QAngle(0,0,0),255,255,0,L4DEspPZAlpha.GetInt(),0);break;
												}
											}
										}
										case ZC_BOOMER:
										{
											if(*PZ_PlayerHealth>2){
												if(isGhost){
													debug->AddBoxOverlay(Orign,Vector(-20,-20,0),Vector(20,20,75),QAngle(0,0,0),255,170,60,5,0);break;
												}
												else{
													debug->AddLineOverlay(GetOwnerPos(),Orign,255,176,60,true,0);debug->AddBoxOverlay(Orign,Vector(-20,-20,0),Vector(20,20,BoxZ),QAngle(0,0,0),255,170,60,L4DEspPZAlpha.GetInt(),0);break;
												}
											}
										}
										case ZC_HUNTER:
										{
											if(*PZ_PlayerHealth>2){
												if(isGhost){
													debug->AddBoxOverlay(Orign,Vector(-20,-20,0),Vector(20,20,75),QAngle(0,0,0),0,150,255,5,0);break;
												}
												else{
													debug->AddLineOverlay(GetOwnerPos(),Orign,0,150,255,true,0);debug->AddBoxOverlay(Orign,Vector(-20,-20,0),Vector(20,20,BoxZ),QAngle(0,0,0),0,150,255,L4DEspPZAlpha.GetInt(),0);break;
												}
											}
										}
										case ZC_TANK:
										{
											if(*PZ_PlayerHealth>2){
												if(isGhost){
													debug->AddBoxOverlay(Orign,Vector(-20,-20,0),Vector(20,20,75),QAngle(0,0,0),0,255,0,5,0);break;
												}
												else{
													debug->AddLineOverlay(GetOwnerPos(),Orign,0,255,0,true,0);debug->AddBoxOverlay(Orign,Vector(-20,-20,0),Vector(20,20,BoxZ+10),QAngle(0,0,0),0,255,0,L4DEspPZAlpha.GetInt(),0);break;
												}
											}
										}
										default: 
											{
											}
									}
								}
							}
						}
					}
				}
				//Esp Spectators
				C_BaseEntity *spect = NULL;
				for(int i=0; i<max_entitys; i++){
					if(isOwnner(i)) continue;
					cl_ent = entList->GetClientEntity(i);
					if(cl_ent!=null){
						if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"CTerrorPlayer")){
							spect = cl_ent->GetBaseEntity();
							int *SpecTeam = (int*) ( ( DWORD )spect + ( DWORD )0x0BC );
							if(*SpecTeam==Spectator){
								debug->AddBoxOverlay(GetEntityOrign(spect),Vector(-5,-5,0),Vector(5,5,10),QAngle(0,0,0),255,255,255,5,0);
							}
						}
					}
				}
			}


			//Esp Infected
			if(L4DEspInfected.GetBool()&&client->IsInGame()){
				int max_entitys = entList->GetMaxEntities();
				IClientEntity *cl_ent;
				C_BaseEntity *Infected_z = NULL;
				CBaseEntity *Owner = GetOwnerEntity();
				int *OwnerTargetID;
				int *m_clientLookatTarget;				

				for(int i=0; i<max_entitys; i++){				
					cl_ent = entList->GetClientEntity(i);
					if (cl_ent==NULL||cl_ent->IsDormant()) continue;
					if(cl_ent!=null){
						if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Infected")){
							Infected_z = cl_ent->GetBaseEntity();
							if(!Infected_z) continue;
							OwnerTargetID = (int*) ( ( DWORD )Owner + ( DWORD )0x150 );
							m_clientLookatTarget = (int*) ( ( DWORD )Infected_z + ( DWORD )0xF14 );

							Vector Orign,Head;
							Orign.Init();
							Head.Init();
							vec_t BoxZ;
							ICollideable *pCollideable = NULL;
							vcollide_t *pCollide = NULL;
							int pColideFlags;
								
							Vector BposA,BposB;
							BposA.Init();
							BposB.Init();

							if(GetEntityOrignP(Infected_z,&Orign)&&GetBoneOrignP(Infected_z,&Head,14)){
								BoxZ = (Head.z - Orign.z) + 10;
								pCollideable = Infected_z->GetCollideable();
								if(!pCollideable) continue;
								pCollide = model_info->GetVCollide(pCollideable->GetCollisionModel());
								if(!pCollide) continue;
								pColideFlags = pCollideable->GetSolidFlags();
								
								if(L4DEspInfected.GetInt()==1&&pColideFlags!=20){
									if(*m_clientLookatTarget==-1){
										debug->AddBoxOverlay(Orign,Vector(-15,-15,0),Vector(15,15,BoxZ),QAngle(0,0,0),255,255,255,5,0);
									}
									else{
										if(*m_clientLookatTarget==*OwnerTargetID){
											debug->AddBoxOverlay(Orign,Vector(-15,-15,0),Vector(15,15,BoxZ),QAngle(0,0,0),255,0,0,5,0);
											if(L4DEspInfected.GetInt()>1) debug->AddLineOverlay(Orign,GetOwnerPos(),255,0,0,true,0);
										}
										else{
											debug->AddBoxOverlay(Orign,Vector(-15,-15,0),Vector(15,15,BoxZ),QAngle(0,0,0),255,0,255,5,0);
										}
									}
								}
							}
						}
					}
				}
			}
			
			//Show Items
			if(ShowItems.GetBool()&&client->IsInGame()){
				int max_entitys = entList->GetMaxEntities();
				int m_WeaponID;
				IClientEntity *cl_ent;
				C_BaseEntity *pEntity = NULL;

				for(int i=0; i<max_entitys; i++){
					cl_ent = entList->GetClientEntity(i);
					if(cl_ent!=null){
						if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"CWeaponSpawn")&&(ShowItems.GetInt()==1||ShowItems.GetInt()==3)){
							pEntity = cl_ent->GetBaseEntity();
							m_WeaponID = *(int*)((char *)pEntity + 2288);
							switch(m_WeaponID){
								case Pistol:
								{
									vphysics->AddEntityTextOverlay(i,-1,0,218,255,127,255,"Pistol.(spawn)");break;
								}
								{
									vphysics->AddEntityTextOverlay(i,-1,0,218,255,127,255,"Smg.(spawn)");break;
								}
								case PumpShotgun:
								{
									vphysics->AddEntityTextOverlay(i,-1,0,182,255,0,255,"PumpShotgun.(spawn)");break;
								}
								case AutoShotgun:
								{
									vphysics->AddEntityTextOverlay(i,-1,0,76,255,0,255,"AutoShotgun.(spawn)");break;
								}
								case Rife:
								{
									vphysics->AddEntityTextOverlay(i,-1,0,91,127,0,255,"Rife.(spawn)");break;
								}
								case HuntingRife:
								{
									vphysics->AddEntityTextOverlay(i,-1,0,255,0,220,255,"HuntingRife.(spawn)");break;
								}
								case MedKit:
								{
									vphysics->AddEntityTextOverlay(i,-1,0,150,150,255,255,"MedKit.(spawn)");break;
								}
								case Molotov:
								{
									vphysics->AddEntityTextOverlay(i,-1,0,255,150,0,255,"Molotov.(spawn)");break;
								}
								case PipeBomb:
								{
									vphysics->AddEntityTextOverlay(i,-1,0,0,255,255,255,"PipeBomb.(spawn)");break;
								}
								case PainPills:
								{
									vphysics->AddEntityTextOverlay(i,-1,0,255,255,0,255,"PainPills.(spawn)");break;
								}
								default:
								{
								}
							}
						}
						if(ShowItems.GetInt()==1||ShowItems.GetInt()==3){
							if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"CWeaponAmmoSpawn")&&(ShowItems.GetInt()==1||ShowItems.GetInt()==3)){
								pEntity = cl_ent->GetBaseEntity();
								vphysics->AddEntityTextOverlay(i,-1,0,255,255,255,255,"AmmoSpawn");
							}
							if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"CPistol")&&(ShowItems.GetInt()==2||ShowItems.GetInt()==3)){
								pEntity = cl_ent->GetBaseEntity();
								vphysics->AddEntityTextOverlay(i,-1,0,218,255,127,255,"Pistol");
							}
							if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"CSubMachinegun")&&(ShowItems.GetInt()==2||ShowItems.GetInt()==3)){
								pEntity = cl_ent->GetBaseEntity();
								vphysics->AddEntityTextOverlay(i,-1,0,218,255,127,255,"Smg");
							}
							if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"CPumpShotgun")&&(ShowItems.GetInt()==2||ShowItems.GetInt()==3)){
								pEntity = cl_ent->GetBaseEntity();
								vphysics->AddEntityTextOverlay(i,-1,0,182,255,0,255,"PumpShotgun");
							}
							if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"CAutoShotgun")&&(ShowItems.GetInt()==2||ShowItems.GetInt()==3)){
								pEntity = cl_ent->GetBaseEntity();
								vphysics->AddEntityTextOverlay(i,-1,0,76,255,0,255,"AutoShotgun");
							}
							if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"CAssaultRifle")&&(ShowItems.GetInt()==2||ShowItems.GetInt()==3)){
								pEntity = cl_ent->GetBaseEntity();
								vphysics->AddEntityTextOverlay(i,-1,0,91,127,0,255,"Rife");
							}
							if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"CSniperRifle")&&(ShowItems.GetInt()==2||ShowItems.GetInt()==3)){
								pEntity = cl_ent->GetBaseEntity();
								vphysics->AddEntityTextOverlay(i,-1,0,255,0,220,255,"HuntingRife");
							}
							if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"CFirstAidKit")&&(ShowItems.GetInt()==2||ShowItems.GetInt()==3)){
								pEntity = cl_ent->GetBaseEntity();
								vphysics->AddEntityTextOverlay(i,-1,0,150,150,255,255,"MedKit");
							}
							if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"CMolotov")&&(ShowItems.GetInt()==2||ShowItems.GetInt()==3)){
								pEntity = cl_ent->GetBaseEntity();
								vphysics->AddEntityTextOverlay(i,-1,0,255,150,0,255,"Molotov");
							}
							if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"CPipeBomb")&&(ShowItems.GetInt()==2||ShowItems.GetInt()==3)){
								pEntity = cl_ent->GetBaseEntity();
								vphysics->AddEntityTextOverlay(i,-1,0,0,255,255,255,"PipeBomb");
							}
							if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"CPainPills")&&(ShowItems.GetInt()==2||ShowItems.GetInt()==3)){
								pEntity = cl_ent->GetBaseEntity();
								vphysics->AddEntityTextOverlay(i,-1,0,255,255,0,255,"PainPills");
							}
						}
					}
				}
			}
			//Tank Helpers
			if(TankHelpers.GetBool()&&client->IsInGame()){
				int max_entitys = entList->GetMaxEntities();
				IClientEntity *cl_ent;
				C_BaseEntity *prop = NULL;
				CBaseEntity *Owner = GetOwnerEntity();
				int Owner_PZ = *(int*)((char *)Owner + 6500);
				if(Owner_PZ==ZC_TANK){
					for(int i=0; i<max_entitys; i++){
						cl_ent = entList->GetClientEntity(i);
						if(cl_ent!=null){
							if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"CPhysicsProp")){
								bool m_hasTankGlow = *(bool*)((char *)cl_ent->GetBaseEntity() + 2298);
								if(m_hasTankGlow) vphysics->AddEntityTextOverlay(i,-1,0,255,255,0,150,"TankProp");
							}
							if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"CMolotov")){
								vphysics->AddEntityTextOverlay(i,-1,0,120,100,255,150,"Molot");
							}
						}
					}
				}
			}

			//Show players health - PZs
			if(ShowPlayersHealth.GetBool()&&client->IsInGame()){
				int max_entitys = entList->GetMaxEntities();
				IClientEntity *cl_ent;
				C_BaseEntity *pEntity = NULL;
				int PZ_PlayerClass;
				int *PZ_PlayerHealth;
				int PZ_InitialHealth;
				bool isGhost = false;
				if(ShowPlayersHealth.GetInt()==1){
					for(int i=0; i<max_entitys; i++){
						if(isOwnner(i)) continue;
						cl_ent = entList->GetClientEntity(i);
						if(cl_ent!=null){
							if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"CTerrorPlayer")){
								pEntity = cl_ent->GetBaseEntity();
								int *PlayerTeam = (int*) ( ( DWORD )pEntity + ( DWORD )0x0BC );
								if(*PlayerTeam==Infected){
									PZ_PlayerClass = *(int*)((char *)pEntity + 6500);
									PZ_PlayerHealth = (int*) ( ( DWORD )pEntity + ( DWORD )0xC4 );
									PZ_InitialHealth = *(int *)((char *)pEntity + 7336);
									isGhost = *(bool *)((char *)pEntity + 6511);
									switch(PZ_PlayerClass){
										case ZC_SMOKER:
										{
											if(*PZ_PlayerHealth>2){
												if(isGhost){
													vphysics->AddEntityTextOverlay(i,-11,0,255,255,255,255,"Smoker");break;
												}
												else{
													vphysics->AddEntityTextOverlay(i,-11,0,255,255,0,255,"Smoker - HP: %d",*PZ_PlayerHealth);break;
												}
											}
										}
										case ZC_BOOMER:
										{
											if(*PZ_PlayerHealth>2){
												if(isGhost){
													vphysics->AddEntityTextOverlay(i,-10,0,255,255,255,255,"Boomer");break;
												}
												else{
													vphysics->AddEntityTextOverlay(i,-10,0,255,170,60,255,"Boomer - HP: %d",*PZ_PlayerHealth);break;
												}
											}
										}
										case ZC_HUNTER:
										{
											if(*PZ_PlayerHealth>2){
												if(isGhost){
													vphysics->AddEntityTextOverlay(i,-8,0,255,255,255,255,"Hunter");break;
												}
												else{
													vphysics->AddEntityTextOverlay(i,-8,0,0,150,255,255,"Hunter - HP: %d",*PZ_PlayerHealth);break;
												}
											}
										}
										case ZC_TANK:
										{
											if(*PZ_PlayerHealth>2){
												if(isGhost){
													vphysics->AddEntityTextOverlay(i,-11,0,255,255,255,255,"Tank");break;

												}else{
													int Tank_HealthPercentage = *PZ_PlayerHealth/((float)PZ_InitialHealth/100);
													vphysics->AddEntityTextOverlay(i,-11,0,0,255,0,255,"Tank   -   [%d <-> %d]",*PZ_PlayerHealth,Tank_HealthPercentage);Draw_PZTank_HealthBar(Tank_HealthPercentage,i,-10,0,0,255,0,255);break;
												}
											}
										}
										default: {
										}
									}
								}
							}
						}
					}
					//Bots_PZ
					int PZ_BotClass;
					int *PZ_BotHealth;
					for(int i=0; i<max_entitys; i++){
						cl_ent = entList->GetClientEntity(i);
						if(cl_ent!=null){
							if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Smoker")||
							!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Boomer")||
							!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Hunter")||
							!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Tank")){
								pEntity = cl_ent->GetBaseEntity();
								PZ_BotClass = *(int *)((char *)pEntity + 6500);
								int *PZ_BotHealth = (int*) ( ( DWORD )pEntity + ( DWORD )0xC4 );
								int PZ_InitialHealth = *(int *)((char *)pEntity + 7336);
								switch(PZ_BotClass){
									case ZC_SMOKER:
									{
										if(*PZ_BotHealth>2){
											vphysics->AddEntityTextOverlay(i,-11,0,255,255,0,255,"Smoker(Bot) - HP: %d",*PZ_BotHealth);break;
										}
									}
									case ZC_BOOMER:
									{
										if(*PZ_BotHealth>2){
											vphysics->AddEntityTextOverlay(i,-10,0,255,170,60,255,"Booer(Bot) - HP: %d",*PZ_BotHealth);break;
										}
									}
									case ZC_HUNTER:
									{
										if(*PZ_BotHealth>2){
											vphysics->AddEntityTextOverlay(i,-8,0,0,150,255,255,"Hunter(Bot) - HP: %d",*PZ_BotHealth);break;
										}
									}
									case ZC_TANK:
									{
										if(*PZ_BotHealth>2){
											int Tank_HealthPercentage = *PZ_BotHealth/((float)PZ_InitialHealth/100);
											vphysics->AddEntityTextOverlay(i,-11,0,0,255,0,255,"Tank(Bot) - [%d <-> %d]",*PZ_BotHealth,Tank_HealthPercentage);Draw_PZTank_HealthBar(Tank_HealthPercentage,i,-10,0,0,255,0,255);break;
										} 
									}
									default:
									{
									}
								}
							}
						}
					}
					//Witch
					for(int i=0; i<max_entitys; i++){
						cl_ent = entList->GetClientEntity(i);
						if(cl_ent!=null){
							if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Witch")){
								pEntity = cl_ent->GetBaseEntity();
								float *m_rage = (float*) ( ( DWORD )pEntity + ( DWORD )0xF24 );
								if(*m_rage!=1.0){
									vphysics->AddEntityTextOverlay(i,-4,0,255,0,220,255,"- Witch -");
									vphysics->AddEntityTextOverlay(i,-3,0,255,255-(*m_rage*255),255-(*m_rage*255),255,"Calm:[%1.1f]",*m_rage);
								}
								else{
									vphysics->AddEntityTextOverlay(i,-3,0,255,0,0,255,"- Rage! -");
								}
							}
						}
					}
				}
				//Spectators
				if(ShowPlayersHealth.GetInt()==3&&client->IsInGame()){
					for(int i=0; i<max_entitys; i++){
						cl_ent = entList->GetClientEntity(i);
						if(cl_ent!=null){
							if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"CTerrorPlayer")){
								pEntity = cl_ent->GetBaseEntity();
								int *PlayerTeam = (int*) ( ( DWORD )pEntity + ( DWORD )0x0BC );
								if(*PlayerTeam==Spectator){
									vphysics->AddEntityTextOverlay(i,-4,0,255,255,255,255,"Spectator");
								}
							}
						}
					}
				}
			}
			/*
					//Survivor_players
					for(int i=0; i<max_entitys; i++){
						cl_ent = entList->GetClientEntity(i);
						if(cl_ent!=null){
							if(strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"CTerrorPlayer")==0){
								pEntity = cl_ent->GetBaseEntity();
								int *SurvivorHealth;
								int *PlayerTeam = (int*) ( ( DWORD )pEntity + ( DWORD )0x0BC );
								SurvivorHealth = (int*) ( ( DWORD )pEntity + ( DWORD )0xC4 );
								if(*PlayerTeam==Survivor){
									vphysics->AddEntityTextOverlay(i,-8,0,0,255,0,255,"Survivor - HP: %d",*SurvivorHealth);break;
								}
							}
						}
					}*/

			//Draw PZ glow
			if(DrawPZGlow.GetBool()&&client->IsInGame()){
				IClientEntity *cl_ent;
				C_BaseEntity *PZ = NULL;
				int max_entitys = entList->GetMaxEntities();
				for(int i=0; i<max_entitys; i++){
					if(isOwnner(i)) continue;
					cl_ent = entList->GetClientEntity(i);
					if(cl_ent!=null){
						if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"CTerrorPlayer")){
							PZ = cl_ent->GetBaseEntity();
							int *m_iTeamNum = (int*) ( ( DWORD )PZ + ( DWORD )0xBC );
							int PZ_Class = *(int *)((char *)PZ + 6500);
							if(DrawPZGlow.GetInt()==1){
								if(PZ_Class==ZC_SMOKER||
								PZ_Class==ZC_BOOMER||
								PZ_Class==ZC_HUNTER||
								PZ_Class==ZC_TANK){
									if(*m_iTeamNum == 3) *m_iTeamNum = 2;
								}
							}
							if(DrawPZGlow.GetInt()==2){
								if(PZ_Class==ZC_SMOKER||
								PZ_Class==ZC_BOOMER||
								PZ_Class==ZC_HUNTER||
								PZ_Class==ZC_TANK){
									if(*m_iTeamNum == 2) *m_iTeamNum = 3;
								}
							}
						}
					}
				}
			}

			//AimBot Players
			if(L4DAimBotPZ.GetBool()&&client->IsInGame()){
				//Aim Mode 1
				if(L4DAimBotPZ.GetInt()==1||L4DAimBotPZ.GetInt()==3){
					int TargetBone = L4DAimBotPZTargetBone.GetInt();
					engine_sv->ServerCommand("l4d_get_owner_aim_pos\n");
					if(GetAsyncKeyState((DWORD)L4DAimBotPZUseKey.GetInt())&&!ienginetool->IsConsoleVisible()&&!GetAsyncKeyState((DWORD)L4DAimBotPZBreakKey.GetInt())){
						engine_sv->ServerCommand("l4d_get_owner_aim_pos\n");
						int max_entitys = entList->GetMaxEntities();
						int *EnemyHealth;
						int *MaxBones;
						int *PlayerTeam;
						float *m_rage;
						bool *isGhost;
						IClientEntity *cl_ent;
						C_BaseEntity *Enemy = NULL;

						Vector EnemyBonePos, OwnerPos, AimVector;
						QAngle AimAngle;

						EnemyBonePos.Init();
						OwnerPos.Init();
						AimVector.Init();
						AimAngle.Init();

						for(int i=0; i<max_entitys; i++){
							cl_ent = entList->GetClientEntity(i);
							if (cl_ent==NULL||cl_ent->IsDormant()) continue;
							if(cl_ent!=null){
								if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"CTerrorPlayer")||
								!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"SurvivorBot")||
								!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Hunter")||
								!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Smoker")||
								!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Boomer")||
								!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Tank")){
									Enemy = cl_ent->GetBaseEntity();
									EnemyHealth = (int*) ( ( DWORD )Enemy + ( DWORD )0xC4 );
									MaxBones = (int*) ( ( DWORD )Enemy + ( DWORD )0x8A0 );
									PlayerTeam = (int*) ( ( DWORD )Enemy + ( DWORD )0x0BC );
									isGhost = (bool*) ( ( DWORD )Enemy + ( DWORD )0x196F );
									Distance = (OwnerAimPos-GetEntityOrign(Enemy)).Length();
									if(TargetBone>*MaxBones) TargetBone = 1;
									if(*PlayerTeam!=GetOwnerTeam()&&*PlayerTeam!=Spectator&&!*isGhost&&Distance<L4DAimBotPZRadius.GetInt()&&*EnemyHealth>2){
										GetBonePosition(Enemy,&EnemyBonePos,TargetBone);
										GetOwnerPos2(&OwnerPos);
										AimVector = (EnemyBonePos-(OwnerPos+Vector(0,0,62)));
										VectorAngles(AimVector,AimAngle);
										engine_cl->SetViewAngles(AimAngle);
										debug->AddBoxOverlay(EnemyBonePos,Vector(-1,-1,-1),Vector(1,1,1),QAngle(0,0,0),0,255,0,255,0);
										DrawCircle(GetEntityOrign(Enemy),QAngle(-90,0,0),12,255,255,0,255,true,0);
									}
								}
							}
						}
						//m_rage = (float*) ( ( DWORD )Enemy + ( DWORD )0xF24 );
					}
					else{
						Distance = 0;
					}
				}
				//Aim Mode 2
				if(L4DAimBotPZ.GetInt()==2||L4DAimBotPZ.GetInt()==3){
					int TargetBone = L4DAimBotPZTargetBone.GetInt();
					C_BaseEntity *Enemy = NULL;
					int *EnemyHealth;
					int *MaxBones;
					int *PlayerTeam;
					float *m_rage;
					Vector EnemyBonePos, OwnerPos, AimVector;
					QAngle AimAngle;

					EnemyBonePos.Init();
					OwnerPos.Init();
					AimVector.Init();
					AimAngle.Init();

					if(GetAsyncKeyState((DWORD)L4DAimBotPZUseKey.GetInt())&&!ienginetool->IsConsoleVisible()){
						if(!EntID) engine_sv->ServerCommand("l4d_get_owner_aim_ent_id\n");
						if(EntID>0){
							if(!strcmp(entList->GetClientEntity(EntID)->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Hunter")||
							!strcmp(entList->GetClientEntity(EntID)->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Smoker")||
							!strcmp(entList->GetClientEntity(EntID)->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Boomer")||
							!strcmp(entList->GetClientEntity(EntID)->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Tank")){
								Enemy = entList->GetClientEntity(EntID)->GetBaseEntity();
								EnemyHealth = (int*) ( ( DWORD )Enemy + ( DWORD )0xC4 );
								MaxBones = (int*) ( ( DWORD )Enemy + ( DWORD )0x8A0 );
								if(*EnemyHealth>2&&GetOwnerTeam()!=3){
									if(TargetBone>*MaxBones) TargetBone = 1;
									GetBonePosition(Enemy,&EnemyBonePos,TargetBone);
									GetOwnerPos2(&OwnerPos);
									AimVector = (EnemyBonePos-(OwnerPos+Vector(0,0,62)));
									VectorAngles(AimVector,AimAngle);
									engine_cl->SetViewAngles(AimAngle);
									debug->AddBoxOverlay(EnemyBonePos,Vector(-1,-1,-1),Vector(1,1,1),QAngle(0,0,0),0,255,0,255,0);
									DrawCircle(GetEntityOrign(Enemy),QAngle(-90,0,0),12,255,255,0,255,true,0);
								}
							}
							if(!strcmp(entList->GetClientEntity(EntID)->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Witch")){
								Enemy = entList->GetClientEntity(EntID)->GetBaseEntity();
								MaxBones = (int*) ( ( DWORD )Enemy + ( DWORD )0x8A0 );
								m_rage = (float*) ( ( DWORD )Enemy + ( DWORD )0xF24 );
								if(GetOwnerTeam()!=3){
									if(TargetBone>*MaxBones) TargetBone = 1;
									GetBonePosition(Enemy,&EnemyBonePos,TargetBone);
									GetOwnerPos2(&OwnerPos);
									AimVector = (EnemyBonePos-(OwnerPos+Vector(0,0,62)));
									VectorAngles(AimVector,AimAngle);
									engine_cl->SetViewAngles(AimAngle);
									debug->AddBoxOverlay(EnemyBonePos,Vector(-1,-1,-1),Vector(1,1,1),QAngle(0,0,0),0,255,0,255,0);
									if(*m_rage!=1.0){
										DrawCircle(GetEntityOrign(Enemy),QAngle(-90,0,0),12,255,255,0,255,true,0);
										vphysics->AddEntityTextOverlay(EntID,-3,0,255,255-(*m_rage*255),255-(*m_rage*255),255,"Calm:[%1.1f]",*m_rage);
									}
									else{
										DrawCircle(GetEntityOrign(Enemy),QAngle(-90,0,0),12,255,0,0,255,true,0);							
									}
								}
							}
							if(!strcmp(entList->GetClientEntity(EntID)->GetBaseEntity()->GetClientClass()->m_pNetworkName,"CTerrorPlayer")||
							!strcmp(entList->GetClientEntity(EntID)->GetBaseEntity()->GetClientClass()->m_pNetworkName,"SurvivorBot")){
								Enemy = entList->GetClientEntity(EntID)->GetBaseEntity();
								EnemyHealth = (int*) ( ( DWORD )Enemy + ( DWORD )0xC4 );
								MaxBones = (int*) ( ( DWORD )Enemy + ( DWORD )0x8A0 );
								PlayerTeam = (int*) ( ( DWORD )Enemy + ( DWORD )0x0BC );
								if(*PlayerTeam!=GetOwnerTeam()&&*EnemyHealth>2){
									if(TargetBone>*MaxBones) TargetBone = 1;
									GetBonePosition(Enemy,&EnemyBonePos,TargetBone);
									GetOwnerPos2(&OwnerPos);
									AimVector = (EnemyBonePos-(OwnerPos+Vector(0,0,62)));
									VectorAngles(AimVector,AimAngle);
									engine_cl->SetViewAngles(AimAngle);
									debug->AddBoxOverlay(EnemyBonePos,Vector(-1,-1,-1),Vector(1,1,1),QAngle(0,0,0),0,255,0,255,0);
									DrawCircle(GetEntityOrign(Enemy),QAngle(-90,0,0),12,255,255,0,255,true,0);
								}
							}
						}
					}
					else{
						EntID = 0;
					}
				}
			}
		
			//AimBot Infected
			if(L4DAimBotInfected.GetBool()&&client->IsInGame()){
				Vector compensation = Vector(
					L4DAimBotInfected_CompensationX.GetFloat(),
					L4DAimBotInfected_CompensationY.GetFloat(),
					L4DAimBotInfected_CompensationZ.GetFloat()
					);
				int TargetBone = L4DAimBotInfectedTargetBone.GetInt();
				if(GetAsyncKeyState((DWORD)L4DAimBotPZUseKey.GetInt())&&!ienginetool->IsConsoleVisible()&&!GetAsyncKeyState((DWORD)L4DAimBotPZBreakKey.GetInt())){
					int max_entitys = entList->GetMaxEntities();
					int *MaxBones;
					IClientEntity *cl_ent;
					C_BaseEntity *Enemy = NULL;

					Vector EnemyBonePos, OwnerPos, AimVector;
					QAngle AimAngle;

					EnemyBonePos.Init();
					OwnerPos.Init();
					AimVector.Init();
					AimAngle.Init();

					ICollideable *pCollideable = NULL;
					vcollide_t *pCollide = NULL;
					int pColideFlags; 

					studiohdr_t *mob_hdr = NULL;
					mstudiobone_t *mob_bone = NULL;

					for(int i=0; i<max_entitys; i++){
						cl_ent = entList->GetClientEntity(i);
						if (cl_ent==NULL||cl_ent->IsDormant()) continue;
						if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Infected")){
							Enemy = cl_ent->GetBaseEntity();
							MaxBones = (int*) ( ( DWORD )Enemy + ( DWORD )0x8A0 );
							Distance_own_inf = (GetEntityOrign(Enemy)-GetOwnerPos()).Length();
							if(!Enemy) continue;
							pCollideable = Enemy->GetCollideable();
							mob_hdr = model_info->GetStudiomodel(GetEntityModel(Enemy));
							if(mob_hdr) mob_bone = mob_hdr->pBone(TargetBone);
							if(!pCollideable) continue;
							pCollide = model_info->GetVCollide(pCollideable->GetCollisionModel());
							if(!pCollide) continue;
							pColideFlags = pCollideable->GetSolidFlags();
							if(TargetBone>*MaxBones) TargetBone = 1;
							if(Distance_own_inf<L4DAimBotInfectedRadius.GetInt()&&pColideFlags!=20){
								if(mob_bone) engine_sv->Con_NXPrintf(&HudMsg(0.9,0.9,0.9,T),"AimBone: %s\n",mob_bone->pszName() );
								GetBonePosition(Enemy,&EnemyBonePos,TargetBone);
								GetOwnerPos2(&OwnerPos);
								AimVector = ((EnemyBonePos+compensation)-(OwnerPos+Vector(0,0,62)));
								VectorAngles(AimVector,AimAngle);
								engine_cl->SetViewAngles(AimAngle);
								switch(L4DAimBotInfectedOverlayMode.GetInt()){
								case 1:
										debug->AddBoxOverlay(GetBoneOrign(Enemy,L4DAimBotInfectedTargetBone.GetInt()),Vector(-1,-1,-1),Vector(1,1,1),QAngle(0,0,0),0,255,0,255,0);
										break;
									case 2:
										debug->AddBoxOverlay(GetBoneOrign(Enemy,L4DAimBotInfectedTargetBone.GetInt()),Vector(-1,-1,-1),Vector(1,1,1),QAngle(0,0,0),0,255,0,255,0);
										debug->AddLineOverlay(GetOwnerPos(),GetBoneOrign(Enemy,0),255,255,0,true,0);
										break;
									case 3:
										debug->AddBoxOverlay(GetEntityOrign(Enemy),Vector(-15,-15,0),Vector(15,15,70),QAngle(0,0,0),255,255,0,0,0);
										break;
									case 4:
										debug->AddBoxOverlay(GetBoneOrign(Enemy,L4DAimBotInfectedTargetBone.GetInt()),Vector(-1,-1,-1),Vector(1,1,1),QAngle(0,0,0),0,255,0,255,0);
										debug->AddBoxOverlay(GetEntityOrign(Enemy),Vector(-15,-15,0),Vector(15,15,70),QAngle(0,0,0),255,255,0,0,0);
										break;
									case 5:
										debug->AddLineOverlay(GetOwnerPos(),GetBoneOrign(Enemy,0),255,255,0,true,0);
										debug->AddBoxOverlay(GetBoneOrign(Enemy,L4DAimBotInfectedTargetBone.GetInt()),Vector(-1,-1,-1),Vector(1,1,1),QAngle(0,0,0),0,255,0,255,0);
										debug->AddBoxOverlay(GetEntityOrign(Enemy),Vector(-15,-15,0),Vector(15,15,70),QAngle(0,0,0),255,255,0,0,0);
										break;
									default:
										debug->AddBoxOverlay(GetBoneOrign(Enemy,L4DAimBotInfectedTargetBone.GetInt()),Vector(-1,-1,-1),Vector(1,1,1),QAngle(0,0,0),0,255,0,255,0);
										break;
									}
								}
							}
						}
					}
					else{
						Distance_own_inf = 0;
					}
			}

			//Hunter Distance Meter
			if(L4DHunterDistanceMeter.GetBool()&&client->IsInGame()){
				engine_sv->ServerCommand("l4d_get_owner_aim_pos\n");
				QAngle eyes_angles;
				engine_cl->GetViewAngles(eyes_angles);
				if(L4DHunterDistanceMeter.GetInt()==1){
					if(L4DHunterDistanceMeterAngle.GetInt()!=0){
						if(eyes_angles.x<L4DHunterDistanceMeterAngle.GetInt()){ 
							debug->AddBoxOverlay(OwnerAimPos-Vector(0,0,11),Vector(-10,-10,-10),Vector(10,10,10),QAngle(0,0,0),0,255,0,5,0);
							debug->AddTextOverlay(OwnerAimPos-Vector(0,0,-20),0,"Dis: %1.0f\n",(OwnerAimPos.z-GetOwnerPos().z));
						}
					}
					if(L4DHunterDistanceMeterAngle.GetInt()==0){
						debug->AddBoxOverlay(OwnerAimPos-Vector(0,0,11),Vector(-10,-10,-10),Vector(10,10,10),QAngle(0,0,0),0,255,0,5,0);
						debug->AddTextOverlay(OwnerAimPos-Vector(0,0,-20),0,"Dis: %1.0f\n",(OwnerAimPos-GetOwnerPos()).Length());
					}
				}
				if(L4DHunterDistanceMeter.GetInt()==2){
					if(L4DHunterDistanceMeterAngle.GetInt()!=0){
						if(eyes_angles.x<L4DHunterDistanceMeterAngle.GetInt()){ 
							debug->AddTextOverlay(OwnerAimPos-Vector(0,0,-20),0,"Dis: %1.0f\n",(OwnerAimPos.z-GetOwnerPos().z));
						}
					}
					if(L4DHunterDistanceMeterAngle.GetInt()==0){
						debug->AddTextOverlay(OwnerAimPos-Vector(0,0,-20),0,"Dis: %1.0f\n",(OwnerAimPos-GetOwnerPos()).Length());
					}
				}
			}

			//Draw View Line
			if(DrawViewLine.GetBool()&&client->IsInGame()){
				CBaseEntity *pEntity = NULL;
				Vector eye_position;
				Vector aim_dir;
				QAngle eye_angles;

				pEntity = GetOwnerEntity();
				eye_position = GetOwnerPos();
				engine_cl->GetViewAngles(eye_angles);
				AngleVectors(eye_angles, &aim_dir);
				VectorNormalize(aim_dir);
				Vector vec_end = eye_position + aim_dir * 8000;
				debug->AddLineOverlay(GetOwnerPos(),vec_end,255,255,255,true,0);
			}
			
			//Test 2
			if(L4DTest2.GetBool()&&client->IsInGame()){
				//engine_cl->ClientCmd("l4d_test\n");

				//matrix3x4_t bone_matrix[MAXSTUDIOBONES];
				//if(entList->GetClientEntity(2)->GetClientRenderable()->SetupBones( bone_matrix, 128, BONE_USED_BY_HITBOX, client->Time())){
				//}
			}
		}
}


DWORD WINAPI L4D_Drawing_Thread (LPVOID){
	for(;;Sleep(1)){
		if(DrawSkeletonInfected.GetBool()){
		}
	}
}


class L4D_VT: public IServerPluginCallbacks, public IGameEventListener
{
public:
	L4D_VT();
	~L4D_VT();

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
	virtual void			ClientPutInServer( edict_t *pEntity, char const *playername);
	virtual void			SetCommandClient( int index );
	virtual void			ClientSettingsChanged( edict_t *pEdict );
	virtual PLUGIN_RESULT	ClientConnect( bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen );
	virtual PLUGIN_RESULT	ClientCommand( edict_t *pEntity, const CCommand &args );
	virtual PLUGIN_RESULT	NetworkIDValidated( const char *pszUserName, const char *pszNetworkID );
	virtual void			OnQueryCvarValueFinished( QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue );

	// IGameEventListener Interface
	virtual void FireGameEvent( KeyValues * event );

	virtual int GetCommandIndex() { return m_iClientCommandIndex; }

private:
	int m_iClientCommandIndex;
};

// 
// The plugin is a static singleton that is exported as an interface
//
L4D_VT g_L4D_VT;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(L4D_VT, IServerPluginCallbacks, INTERFACEVERSION_ISERVERPLUGINCALLBACKS, g_L4D_VT );

//---------------------------------------------------------------------------------
// Purpose: constructor/destructor
//---------------------------------------------------------------------------------
L4D_VT::L4D_VT()
{
	m_iClientCommandIndex = 0;
}

L4D_VT::~L4D_VT()
{
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is loaded, load the interface we need from the engine
//---------------------------------------------------------------------------------
bool L4D_VT::Load(	CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory )
{
	ConnectTier1Libraries( &interfaceFactory, 1 );
	ConnectTier2Libraries( &interfaceFactory, 1 );

	CreateInterfaceFn clientFactory = (CreateInterfaceFn)GetProcAddress(GetModuleHandleA("client.dll"),"CreateInterface");
	CreateInterfaceFn serverFactory = (CreateInterfaceFn)GetProcAddress(GetModuleHandleA("server.dll"),"CreateInterface");
	CreateInterfaceFn materialFactory = (CreateInterfaceFn)GetProcAddress(GetModuleHandleA("materialsystem.dll"),"CreateInterface");
	CreateInterfaceFn datacacheFactory = (CreateInterfaceFn)GetProcAddress(GetModuleHandleA("datacache.dll"),"CreateInterface");
	CreateInterfaceFn stdRenderFactory = (CreateInterfaceFn)GetProcAddress(GetModuleHandleA("studiorender.dll"),"CreateInterface");
	CreateInterfaceFn vguimatsurfaceFactory = (CreateInterfaceFn)GetProcAddress(GetModuleHandleA("vguimatsurface.dll"),"CreateInterface");
	CreateInterfaceFn steamclientFactory = (CreateInterfaceFn)GetProcAddress(GetModuleHandleA("steamclient.dll"), "CreateInterface");

	engine_sv = (IVEngineServer*)interfaceFactory(INTERFACEVERSION_VENGINESERVER, NULL);
	engine_cl = (IVEngineClient*)interfaceFactory(VENGINE_CLIENT_INTERFACE_VERSION, NULL);
	entList = (IClientEntityList*)clientFactory(VCLIENTENTITYLIST_INTERFACE_VERSION, NULL);
	vphysics = (IVPhysicsDebugOverlay*)interfaceFactory(VPHYSICS_DEBUG_OVERLAY_INTERFACE_VERSION, NULL);
	playerinfomanager = (IPlayerInfoManager *)gameServerFactory(INTERFACEVERSION_PLAYERINFOMANAGER,NULL);
	enginetrace = (IEngineTrace *)interfaceFactory(INTERFACEVERSION_ENGINETRACE_CLIENT,NULL);
	client = (IVEngineClient*)interfaceFactory(VENGINE_CLIENT_INTERFACE_VERSION, NULL);
	ienginetool = (IEngineTool*)interfaceFactory(VENGINETOOL_INTERFACE_VERSION, NULL);
	materials = (IMaterialSystem*)materialFactory("VMaterialSystem080", NULL);
	clientDLL = (IBaseClientDLL*)clientFactory("VClient016", NULL);
	gamedll = (IServerGameDLL*)serverFactory("ServerGameDLL005", NULL);
	sv_gcl = (IServerGameClients*)serverFactory(INTERFACEVERSION_SERVERGAMECLIENTS, NULL);
	g_pCVar = (ICvar*)interfaceFactory(CVAR_INTERFACE_VERSION, NULL);
	cl_gameeventmanager = (IGameEventManager*)interfaceFactory(INTERFACEVERSION_GAMEEVENTSMANAGER,NULL);
	sv_ents = (IServerGameEnts*)serverFactory("ServerGameEnts001",NULL);
	server_tool = (IServerTools*)serverFactory(VSERVERTOOLS_INTERFACE_VERSION,NULL);
	client_tool = (IClientTools*)clientFactory(VCLIENTTOOLS_INTERFACE_VERSION,NULL);
	debug = (IVDebugOverlay*)interfaceFactory(VDEBUG_OVERLAY_INTERFACE_VERSION, NULL);
	model_render = (IVModelRender*)interfaceFactory("VEngineModel016", NULL);
	model_info = (IVModelInfo*)interfaceFactory("VModelInfoClient004", NULL);
	physOBJ = (IPhysicsObject*)interfaceFactory(VPHYSICS_INTERFACE_VERSION, NULL);
	mdl_cache = (IMDLCache*)datacacheFactory("MDLCache004", NULL);
	std_render = (IStudioRender*)stdRenderFactory("VStudioRender026", NULL);
	surface = (vgui::ISurface*)vguimatsurfaceFactory("VGUI_Surface030", NULL);
	mdlcache = (IMDLCache*)interfaceFactory("MDLCache004", NULL);

	gameServer = (ISteamGameServer009*)steamclientFactory("SteamClient009", NULL);	
	
	if ( playerinfomanager )
	{
		globals = playerinfomanager->GetGlobalVars();
	}

	cl_gameeventmanager->AddListener(this,false);

	/*engine->GetGameDir(gamedir,sizeof(gamedir));
	engine->GetGameDir(gamedir_demos,sizeof(gamedir_demos));
	mkdir(strcat(gamedir_demos,"/demos"));
	Unlock_ConVars();*/

	ConVar_Register( 0 );
	ConColorMsg(Color(0,255,0,255),"L4D Versus Tool Loaded Successfully.\n");

	l4d_thread = CreateThread(NULL,NULL,L4D_VT_MainThread,NULL,NULL,NULL);

	ShowWindow(FindWindowA("ConsoleWindowClass", NULL), false);
	TargetWnd = FindWindow(0, "Left 4 Dead");
	HDC HDC_Desktop = GetDC(TargetWnd);
	SetupDrawing(HDC_Desktop, TargetWnd);

	return true;
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is unloaded (turned off)
//---------------------------------------------------------------------------------
void L4D_VT::Unload( void )
{
	ConVar_Unregister( );
	DisconnectTier2Libraries( );
	DisconnectTier1Libraries( );
	cl_gameeventmanager->RemoveListener(this);
	isThreadActive = false;
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is paused (i.e should stop running but isn't unloaded)
//---------------------------------------------------------------------------------
void L4D_VT::Pause( void )
{
}

//---------------------------------------------------------------------------------
// Purpose: called when the plugin is unpaused (i.e should start executing again)
//---------------------------------------------------------------------------------
void L4D_VT::UnPause( void )
{
}

//---------------------------------------------------------------------------------
// Purpose: the name of this plugin, returned in "plugin_print" command
//---------------------------------------------------------------------------------
const char *L4D_VT::GetPluginDescription( void )
{
	return "L4D_Versus_Tool v 2.0, Senny";
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void L4D_VT::LevelInit( char const *pMapName )
{
}

//---------------------------------------------------------------------------------
// Purpose: called on level start, when the server is ready to accept client connections
//		edictCount is the number of entities in the level, clientMax is the max client count
//---------------------------------------------------------------------------------
void L4D_VT::ServerActivate( edict_t *pEdictList, int edictCount, int clientMax )
{
}

//---------------------------------------------------------------------------------
// Purpose: called once per server frame, do recurring work here (like checking for timeouts)
//---------------------------------------------------------------------------------

void L4D_VT::GameFrame( bool simulating )
{

}

//---------------------------------------------------------------------------------
// Purpose: called on level end (as the server is shutting down or going to a new map)
//---------------------------------------------------------------------------------
void L4D_VT::LevelShutdown( void ) // !!!!this can get called multiple times per map change
{	
	engine_sv->ServerCommand("l4d_mat_postprocess_enable\n");
	engine_sv->ServerCommand("host_timescale 1.0\n");
	L4DBhop.SetValue(0);
	L4DEspPZ.SetValue(0);
	L4DEspInfected.SetValue(0);
	ShowItems.SetValue(0);
	ShowPlayersHealth.SetValue(0);
	TankHelpers.SetValue(0);
	DrawPZGlow.SetValue(0);
	L4DAimBotPZ.SetValue(0);
	L4DAimBotInfected.SetValue(0);
	DrawViewLine.SetValue(0);
	L4DHunterDistanceMeter.SetValue(0);
	L4DTest2.SetValue(0);
}
//---------------------------------------------------------------------------------
// Purpose: called when a client spawns into a server (i.e as they begin to play)
//---------------------------------------------------------------------------------

void L4D_VT::ClientActive( edict_t *pEntity )
{
}

//---------------------------------------------------------------------------------
// Purpose: called when a client leaves a server (or is timed out)
//---------------------------------------------------------------------------------
void L4D_VT::ClientDisconnect( edict_t *pEntity )
{
}
//---------------------------------------------------------------------------------
// Purpose: called on 
//---------------------------------------------------------------------------------

void L4D_VT::ClientPutInServer( edict_t *pEntity, char const *playername)
{	
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void L4D_VT::SetCommandClient( int index )
{
	m_iClientCommandIndex = index;
}

//---------------------------------------------------------------------------------
// Purpose: called on level start
//---------------------------------------------------------------------------------
void L4D_VT::ClientSettingsChanged( edict_t *pEdict )
{
}

//---------------------------------------------------------------------------------
// Purpose: called when a client joins a server
//---------------------------------------------------------------------------------
PLUGIN_RESULT L4D_VT::ClientConnect( bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen )
{	
	return PLUGIN_CONTINUE;
}

//---------------------------------------------------------------------------------
// Purpose: called when a client types in a command (only a subset of commands however, not CON_COMMAND's)
//---------------------------------------------------------------------------------
PLUGIN_RESULT L4D_VT::ClientCommand( edict_t *pEntity, const CCommand &args )
{
	return PLUGIN_CONTINUE;
}

//---------------------------------------------------------------------------------
// Purpose: called when a client is authenticated
//---------------------------------------------------------------------------------
PLUGIN_RESULT L4D_VT::NetworkIDValidated( const char *pszUserName, const char *pszNetworkID )
{
	return PLUGIN_CONTINUE;
}

//---------------------------------------------------------------------------------
// Purpose: called when a cvar value query is finished
//---------------------------------------------------------------------------------
void L4D_VT::OnQueryCvarValueFinished( QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue )
{
}

//---------------------------------------------------------------------------------
// Purpose: called when an event is fired
//---------------------------------------------------------------------------------
void L4D_VT::FireGameEvent( KeyValues * event )
{
	//Debug events
	if(DebugGameEvents.GetInt()==1){
		const char * name = event->GetName();
		Msg( "\nL4D_VT::FireGameEvent: Got event \"%s\"\n", name );
	}
	if(DebugGameEvents.GetInt()==2){
		const char * name = event->GetName();
		engine_sv->Con_NXPrintf(&HudMsg(0.9,0.9,0.9,5.0),"L4D_VT::FireGameEvent: Got event \"%s\"", name );
	}
	//Chat Listener
	if(ListenChat.GetBool()){
		if (!strcmp(event->GetName(),"player_say")){
			engine_sv->Con_NXPrintf(&HudMsg(0.9,0.9,0.9,5.0),"%d: %s",event->GetInt("userid"),event->GetString("text"));
		}
	}
	//MultyPlayer
	if(AutoRecordDemoMp.GetBool()&&!SP()){
		if (!strcmp(event->GetName(),"server_spawn")){
			RecordDemo();
		}
		if (!strcmp(event->GetName(),"map_transition")){
			engine_sv->ServerCommand("stop\n");
		}
	}
	//SinglePlayer
	if(AutoRecordDemoSp.GetBool()&&SP()){
		if (!strcmp(event->GetName(),"server_spawn")){
			RecordDemo();
		}
		if (!strcmp(event->GetName(),"map_transition")){
			engine_sv->ServerCommand("stop\n");
		}
	}
	//Zombie kill counter
	if(KillCounter.GetInt()){
		IClientEntity* pLocalPlayer = entList->GetClientEntity( client->GetLocalPlayer());
		CBaseEntity *pEntity = pLocalPlayer->GetBaseEntity();
		m_iTeamNum = (int*) ( ( DWORD )pEntity + ( DWORD )0x0BC );

		if(*m_iTeamNum==2){
			if (!strcmp(event->GetName(),"infected_death")){
				if(event->GetInt("attacker")==KillCounter.GetInt()){
					if(event->GetInt("headshot")){
						ZombieCounter(true,true,false);
					}
					else{
						ZombieCounter(true,false,false);
					}
				}
			}
			if (!strcmp(event->GetName(),"player_death")){
				if(event->GetInt("attacker")==KillCounter.GetInt()){
					ZombieCounter(false,false,true);
				}
			}
		}
	}
	//Common events
	if (!strcmp(event->GetName(),"map_transition")){
		zombies = 0;
		headshots = 0;
		players = 0;
	}
	if (!strcmp(event->GetName(),"round_end")){
		zombies = 0;
		headshots = 0;
		players = 0;
	}
}

CON_COMMAND(l4d_show_interfaces , "l4d_show_interfaces.\n"){
	Msg("engine_sv = %x\n",engine_sv);
	Msg("engine_cl = %x\n",engine_cl);
	Msg("entList = %x \n",entList);
	Msg("vphysics = %x\n",vphysics);
	Msg("playerinfomanager = %x\n",playerinfomanager);
	Msg("enginetrace = %x\n",enginetrace);
	Msg("client = %x\n",client);
	Msg("ienginetool %x\n",ienginetool);
	Msg("clientDLL = %x\n",clientDLL);
	Msg("gamedll = %x\n",gamedll);
	Msg("g_pCVar = %x\n",g_pCVar);
	Msg("materials = %x\n",Msg);
	Msg("globals = %x\n",globals);
	Msg("cl_gameeventmanager = %x\n",cl_gameeventmanager);
	Msg("sv_ents = %x\n",sv_ents);
	Msg("server_tool = %x\n",server_tool);
	Msg("client_tool = %x\n",client_tool);
	Msg("debug = %x\n",debug);
	Msg("sv_gcl = %x\n",sv_gcl);
	Msg("model_render = %x\n",model_render);
	Msg("model_info = %x\n",model_info);
	Msg("physOBJ = %x\n",physOBJ);
	Msg("mdl_cache = %x\n",mdl_cache);
	Msg("std_render = %x\n",std_render);
	Msg("surface = %x\n",surface);
	Msg("gameServer = %x\n", gameServer);
}

CON_COMMAND( l4d_versus_tool_version, "Prints the version of the plugin.\n")
{	
	ConColorMsg(Color(0,255,0,255),"L4D Versus Tool by Senny\n");
	ConColorMsg(Color(0,255,0,255),"04.05.2014\n");
	ConColorMsg(Color(0,255,0,255),"Version: 2.0\n");
}

CON_COMMAND(l4d_thirdperson , "Enable thirdperson mode for infected and survivors.\n"){

	l4d_versus_thirdperson_click++;

	if(l4d_versus_thirdperson_click==1){
		ConVar * gmode = NULL;
		ConVar * svc = NULL;
		ConCommand * thp = NULL;
		ConVar * drc2 = NULL;

		gmode = g_pCVar->FindVar("mp_gamemode");
		if(gmode == NULL){
			Warning("Failed to find ConVar mp_gamemode!\n");
			return;
		}

		svc = g_pCVar->FindVar("sv_cheats");
		if(svc == NULL){
			Warning("Failed to find ConVar sv_cheats!\n");
			return;
		}

		thp = g_pCVar->FindCommand("thirdperson");
		if(thp == NULL){
			Warning("Failed to find Command thirdperson!\n");
			return;
		}

		drc2 = g_pCVar->FindVar("demo_recordcommands");
		if(drc2 == NULL){
			Warning("Failed to find Command demo_recordcommands!\n");
			return;
		}

		gmode->RemoveFlags(gmode->GetFlags());
		svc->RemoveFlags(svc->GetFlags());
		thp->RemoveFlags(thp->GetFlags());
		drc2->RemoveFlags(drc2->GetFlags());
		engine_sv->ServerCommand("demo_recordcommands 1\n");
		engine_sv->ServerCommand("mp_gamemode coop\n");
		engine_sv->ServerCommand("sv_cheats 1\n");
		engine_sv->ServerCommand("thirdperson\n");
		engine_sv->ServerCommand("demo_recordcommands 0\n");
	}
	if(l4d_versus_thirdperson_click>1){
		engine_sv->ServerCommand("demo_recordcommands 1\n");
		engine_sv->ServerCommand("thirdperson\n");
		engine_sv->ServerCommand("mp_gamemode versus\n");
		engine_sv->ServerCommand("demo_recordcommands 0\n");
		l4d_versus_thirdperson_click=0;
	}
}

CON_COMMAND(l4d_thirdpersonshoulder , "Enable thirdpersonshoulder mode for infected and survivors.\n"){

	l4d_thirdpersonshoulder_click++;

	if(l4d_thirdpersonshoulder_click==1){
		ConVar * gmode = NULL;
		gmode = g_pCVar->FindVar("mp_gamemode");
		ConVar * drc2 = NULL;

		if(gmode == NULL){
			Warning("Failed to find ConVar mp_gamemode!\n");
			return;
		}

		drc2 = g_pCVar->FindVar("demo_recordcommands");
		if(drc2 == NULL){
			Warning("Failed to find Command demo_recordcommands!\n");
			return;
		}

		gmode->RemoveFlags(gmode->GetFlags());
		drc2->RemoveFlags(drc2->GetFlags());
		engine_sv->ServerCommand("demo_recordcommands 1\n");
		engine_sv->ServerCommand("mp_gamemode coop\n");
		engine_sv->ServerCommand("c_thirdpersonshoulderdist 100\n");
		engine_sv->ServerCommand("thirdpersonshoulder\n");
		engine_sv->ServerCommand("demo_recordcommands 0\n");
	}
	if(l4d_thirdpersonshoulder_click>1){
		engine_sv->ServerCommand("demo_recordcommands 1\n");
		engine_sv->ServerCommand("thirdpersonshoulder\n");
		engine_sv->ServerCommand("mp_gamemode versus\n");
		engine_sv->ServerCommand("demo_recordcommands 0\n");
		l4d_thirdpersonshoulder_click=0;
	}
}

CON_COMMAND(l4d_z_view_distance, "z_view_distance\n"){
	
	l4d_z_view_distance_click++;
	if(l4d_z_view_distance_click==1){
		ConVar * zvd = NULL;
		zvd = g_pCVar->FindVar("z_view_distance");
		if(zvd == NULL){
			Warning("Failed to find Cvar z_view_distance!\n");
			return;
		}
		zvd->RemoveFlags(zvd->GetFlags());
		engine_sv->ServerCommand("z_view_distance -100\n");
	}
	if(l4d_z_view_distance_click>1){
		engine_sv->ServerCommand("z_view_distance 0\n");
		l4d_z_view_distance_click=0;
	}
}

CON_COMMAND(l4d_mat_postprocess_enable, "Disable material postprocess.\n"){

	ConVar * pprocess = NULL;
	pprocess = g_pCVar->FindVar("mat_postprocess_enable");
	if(pprocess == NULL){
		Warning("Failed to Cvar command mat_postprocess_enable!\n");
		return;
	}
	pprocess->RemoveFlags(pprocess->GetFlags());
	engine_sv->ServerCommand("mat_postprocess_enable 0\n");
	engine_sv->Con_NXPrintf(&HudMsg(R,G,B,T),"Mat_postprocess_enable off\n");
}

CON_COMMAND(l4d_r_drawothermodels, "r_drawothermodels\n"){

	l4d_r_drawothermodels_click++;

	if(l4d_r_drawothermodels_click==1){
		ConVar * rdom = NULL;
		rdom = g_pCVar->FindVar("r_drawothermodels");
		if(rdom == NULL){
			Warning("Failed to find Cvar r_drawothermodels!\n");
			return;
		}
		rdom->RemoveFlags(rdom->GetFlags());
		engine_sv->ServerCommand("r_drawothermodels 2\n");
	}
	if(l4d_r_drawothermodels_click>1){
		engine_sv->ServerCommand("r_drawothermodels 1\n");
		l4d_r_drawothermodels_click=0;
	}
}

CON_COMMAND(l4d_r_drawstaticprops, "r_drawstaticprops\n"){
	
	l4d_r_drawstaticprops_click++;
	if(l4d_r_drawstaticprops_click==1){
		ConVar * rdsp = NULL;
		rdsp = g_pCVar->FindVar("r_drawstaticprops");
		if(rdsp == NULL){
			Warning("Failed to find Cvar r_drawstaticprops!\n");
			return;
		}
		rdsp->RemoveFlags(rdsp->GetFlags());
		engine_sv->ServerCommand("r_drawstaticprops 2\n");
	}
	if(l4d_r_drawstaticprops_click>1){
		engine_sv->ServerCommand("r_drawstaticprops 1\n");
		l4d_r_drawstaticprops_click=0;
	}
}

CON_COMMAND(l4d_r_drawworld, "r_drawworld\n"){
	
	l4d_r_drawworld_click++;
	if(l4d_r_drawworld_click==1){
		ConVar * rdw = NULL;
		rdw = g_pCVar->FindVar("r_drawworld");
		if(rdw == NULL){
			Warning("Failed to find Cvar r_drawworld!\n");
			return;
		}
		rdw->RemoveFlags(rdw->GetFlags());
		engine_sv->ServerCommand("r_drawworld 0\n");
	}
	if(l4d_r_drawworld_click>1){
		engine_sv->ServerCommand("r_drawworld 1\n");
		l4d_r_drawworld_click=0;
	}
}

CON_COMMAND(l4d_r_drawclipbrushes, "r_drawclipbrushes\n"){
	
	l4d_r_drawclipbrushes_click++;
	if(l4d_r_drawclipbrushes_click==1){
		ConVar * rdcb = NULL;
		rdcb = g_pCVar->FindVar("r_drawclipbrushes");
		if(rdcb == NULL){
			Warning("Failed to find Cvar r_drawclipbrushes\n");
			return;
		}
		rdcb->RemoveFlags(rdcb->GetFlags());
		engine_sv->ServerCommand("r_drawclipbrushes 2\n");
	}
	if(l4d_r_drawclipbrushes_click>1){
		engine_sv->ServerCommand("r_drawclipbrushes 0\n");
		l4d_r_drawclipbrushes_click=0;
	}
}

CON_COMMAND(l4d_showtriggers_toggle, "showtriggers_toggle\n"){
	
	l4d_showtriggers_toggle_click++;
	if(l4d_showtriggers_toggle_click==1){
		ConCommand * st = NULL;
		st = g_pCVar->FindCommand("showtriggers_toggle");
		if(st == NULL){
			Warning("Failed to find Command showtriggers_toggle!\n");
			return;
		}
		st->RemoveFlags(st->GetFlags());
		engine_sv->ServerCommand("showtriggers_toggle\n");
	}
	if(l4d_showtriggers_toggle_click>1){
		engine_sv->ServerCommand("showtriggers_toggle\n");
		l4d_showtriggers_toggle_click=0;
	}
}

CON_COMMAND(l4d_mat_wireframe , "mat_wireframe.\n"){

	l4d_mat_wireframe_click++;
	if(l4d_mat_wireframe_click==1){
		ConVar * svc = NULL;
		ConVar * mw = NULL;

		svc = g_pCVar->FindVar("sv_cheats");
		if(svc == NULL){
			Warning("Failed to find ConVar sv_cheats!\n");
			return;
		}
		mw = g_pCVar->FindVar("mat_wireframe");
		if(mw == NULL){
			Warning("Failed to find Cvar mat_wireframe!\n");
			return;
		}

		svc->RemoveFlags(svc->GetFlags());
		mw->RemoveFlags(mw->GetFlags());
		engine_sv->ServerCommand("sv_cheats 1\n");
		engine_sv->ServerCommand("mat_wireframe 1\n");
	}
	if(l4d_mat_wireframe_click>1){
		engine_sv->ServerCommand("mat_wireframe 0\n");
		engine_sv->ServerCommand("sv_cheats 0\n");
		l4d_mat_wireframe_click=0;
	}
}

CON_COMMAND(l4d_cl_viewmodelfovsurvivor, "cl_viewmodelfovsurvivor.\n"){
	
	l4d_cl_viewmodelfovsurvivor_click++;
	if(l4d_cl_viewmodelfovsurvivor_click==1){
		ConVar * cvms = NULL;
		cvms = g_pCVar->FindVar("cl_viewmodelfovsurvivor");
		if(cvms == NULL){
			Warning("Failed to find ConVar cl_viewmodelfovsurvivor!\n");
			return;
		}
		cvms->RemoveFlags(cvms->GetFlags());
		engine_sv->ServerCommand("cl_viewmodelfovsurvivor 70.0\n");
	}
	if(l4d_cl_viewmodelfovsurvivor_click>1){
		engine_sv->ServerCommand("cl_viewmodelfovsurvivor 50.0\n");
		l4d_cl_viewmodelfovsurvivor_click=0;
	}
}

CON_COMMAND(l4d_cl_viewmodelfovboomer, "cl_viewmodelfovboomer.\n"){
	
	l4d_cl_viewmodelfovboomer_click++;
	if(l4d_cl_viewmodelfovboomer_click==1){
		ConVar * cvmb = NULL;
		cvmb = g_pCVar->FindVar("cl_viewmodelfovboomer");
		if(cvmb == NULL){
			Warning("Failed to find ConVar cl_viewmodelfovboomer!\n");
			return;
		}
		cvmb->RemoveFlags(cvmb->GetFlags());
		engine_sv->ServerCommand("cl_viewmodelfovboomer 70.0\n");
	}
	if(l4d_cl_viewmodelfovboomer_click>1){
		engine_sv->ServerCommand("cl_viewmodelfovboomer 50\n");
		l4d_cl_viewmodelfovboomer_click=0;
	}
}

CON_COMMAND(l4d_cl_viewmodelfovhunter, "cl_viewmodelfovhunter.\n"){
	
	l4d_cl_viewmodelfovhunter_click++;
	if(l4d_cl_viewmodelfovhunter_click==1){
		ConVar * cvmh = NULL;
		cvmh = g_pCVar->FindVar("cl_viewmodelfovhunter");
		if(cvmh == NULL){
			Warning("Failed to find ConVar cl_viewmodelfovhunter!\n");
			return;
		}
		cvmh->RemoveFlags(cvmh->GetFlags());
		engine_sv->ServerCommand("cl_viewmodelfovhunter 70.0\n");
	}
	if(l4d_cl_viewmodelfovhunter_click>1){
		engine_sv->ServerCommand("cl_viewmodelfovhunter 50\n");
		l4d_cl_viewmodelfovhunter_click=0;
	}
}

CON_COMMAND(l4d_cl_viewmodelfovsmoker, "cl_viewmodelfovsmoker.\n"){
	
	l4d_cl_viewmodelsmoker_click++;
	if(l4d_cl_viewmodelsmoker_click==1){
		ConVar * cvms = NULL;
		cvms = g_pCVar->FindVar("cl_viewmodelfovsmoker");
		if(cvms == NULL){
			Warning("Failed to find ConVar cl_viewmodelfovsmoker!\n");
			return;
		}
		cvms->RemoveFlags(cvms->GetFlags());
		engine_sv->ServerCommand("cl_viewmodelfovsmoker 70.0\n");
	}
	if(l4d_cl_viewmodelsmoker_click>1){
		engine_sv->ServerCommand("cl_viewmodelfovsmoker 50\n");
		l4d_cl_viewmodelsmoker_click=0;
	}
}

CON_COMMAND(l4d_cl_viewmodelfovtank, "cl_viewmodelfovtank.\n"){
	
	l4d_cl_viewmodeltank_click++;
	if(l4d_cl_viewmodeltank_click==1){
		ConVar * cvmt = NULL;
		cvmt = g_pCVar->FindVar("cl_viewmodelfovtank");
		if(cvmt == NULL){
			Warning("Failed to find ConVar cl_viewmodelfovtank!\n");
			return;
		}
		cvmt->RemoveFlags(cvmt->GetFlags());
		engine_sv->ServerCommand("cl_viewmodelfovtank 70.0\n");
	}
	if(l4d_cl_viewmodeltank_click>1){
		engine_sv->ServerCommand("cl_viewmodelfovtank 50\n");
		l4d_cl_viewmodeltank_click=0;
	}
}

CON_COMMAND(l4d_versus_tool_unload, "Unload this plugin\n"){
	PluginUnload();
}

CON_COMMAND(l4d_versu_tool_l4d_cvarlist, "Print and save to file all console commands.\n"){

	FILE *output;
	int cnt=0;

	output=fopen("L4D_CvaList.txt","w");
	ICvar::Iterator iter(g_pCVar);

	for ( iter.SetFirst() ; iter.IsValid() ; iter.Next() )
	{  
		ConCommandBase *cmd = iter.Get();
		cnt++;
		if(strlen(cmd->GetHelpText())>1){
			fprintf(output,"%d - %s //%s\n",cnt,cmd->GetName(),cmd->GetHelpText());
		}
		else{
			fprintf(output,"%d - %s\n",cnt,cmd->GetName());
		}
	}
	cnt=0;
	fclose(output);
	ConColorMsg(Color(0,255,0,255),"Cvars&ConCommands list has been saved to 'L4D_Cvars.txt' in your game folder.\n");
}

CON_COMMAND(l4d_unlock_cvars, "Unlock all console commands & ConVars.\n"){

	ICvar::Iterator iter(g_pCVar);

	for ( iter.SetFirst() ; iter.IsValid() ; iter.Next() )
	{  
		ConCommandBase *cmd = iter.Get();
		cmd->RemoveFlags(FCVAR_DEVELOPMENTONLY);
	}
	ConColorMsg(Color(0,255,0,255),"All Cvars&ConCommands have been unlocked!.\n");
}

CON_COMMAND(l4d_record_demo, "record demo.\n"){

	if(client->IsInGame()){
		record++;
		if(record==1){
			engine_sv->ServerCommand("stop");
			std::string CurrentMapName = "null";
			if(strlen(ienginetool->GetCurrentMap())>0){
				CurrentMapName = std::string(ienginetool->GetCurrentMap());
				CurrentMapName.replace(CurrentMapName.find("maps/"),5,"");
				CurrentMapName.replace(CurrentMapName.find(".bsp"),4,"");
			}

			char* record = "record ";
			char buffer[80];
			time_t seconds = time(NULL);
			tm* timeinfo = localtime(&seconds);
			char* format = "%Y_%m_%d_%H-%M-%S";
			strftime(buffer, 80, format, timeinfo);
		
			std::string str_record = std::string(record);
			std::string str_buffer = std::string(buffer);
			std::string demoname = str_record + CurrentMapName + "_" + str_buffer + "\n";
			std::string hud_info = "Start recording to " + CurrentMapName + "_" + str_buffer + ".dem\n";
		
			ConVar * drc = NULL;
			drc = g_pCVar->FindVar("demo_recordcommands");
			if(drc == NULL){
				Warning("Failed to find ConVar demo_recordcommands!\n");
				return;
			}
			drc->RemoveFlags(drc->GetFlags());

			engine_sv->ServerCommand("demo_recordcommands 0\n");
			engine_sv->ServerCommand(demoname.c_str());

			engine_sv->Con_NXPrintf(&HudMsg(R,G,B,T),hud_info.c_str());
			ConColorMsg(Color(0,128,255,255),hud_info.c_str());
			engine_sv->ServerCommand("playgamesound UI\\BeepClear.wav\n");
		}
		if(record>1){
			engine_sv->ServerCommand("stop\n");

			engine_sv->Con_NXPrintf(&HudMsg(R,G,B,T),"Stop recording demo...\n");
			ConColorMsg(Color(0,128,255,255),"Stop recording demo...\n");
			engine_sv->ServerCommand("playgamesound UI\\BigReward.wav\n");
			record=0;
		}
	}
	else{
		Warning("Gameplay is not running!\n");
	}
}

CON_COMMAND(l4d_host_timescale, "host_timescale.\n"){

	l4d_host_timescale_click++;

	if(l4d_host_timescale_click==1){
		ConVar *svc = NULL;
		ConVar * hts = NULL;

		svc = g_pCVar->FindVar("sv_cheats");
		if(svc == NULL){
			Warning("Failed to find ConVar sv_cheats!\n");
			return;
		}
		svc->RemoveFlags(svc->GetFlags());
		engine_sv->ServerCommand("sv_cheats 1\n");

		hts = g_pCVar->FindVar("host_timescale");
		if(hts == NULL){
			Warning("Failed to find ConVar host_timescale!\n");
			return;
		}
		hts->RemoveFlags(hts->GetFlags());
		engine_sv->ServerCommand("host_timescale 1.2\n");
		engine_sv->Con_NXPrintf(&HudMsg(R,G,B,T),"Host_timescale 1.2\n");
	}
	if(l4d_host_timescale_click==2){
		engine_sv->ServerCommand("host_timescale 1.5\n");
		engine_sv->Con_NXPrintf(&HudMsg(R,G,B,T),"Host_timescale 1.5\n");
	}
	if(l4d_host_timescale_click>2){
		engine_sv->ServerCommand("host_timescale 1.0\n");
		engine_sv->ServerCommand("sv_cheats 0\n");
		l4d_host_timescale_click=0;
		engine_sv->Con_NXPrintf(&HudMsg(R,G,B,T),"Host_timescale 1.0\n");
	}
	//ienginetool->SetTimescale(10);
}

CON_COMMAND(l4d_survivor_upgrades, "survivor_upgrades.\n"){
	
	ConVar * su = NULL;
	su = g_pCVar->FindVar("survivor_upgrades");
	if(su == NULL){
			Warning("Failed to find ConVar Survivor_upgrades!\n");
			return;
		}
	su->RemoveFlags(su->GetFlags());
	engine_sv->ServerCommand("survivor_upgrades 1\n");
	ConColorMsg(Color(0,128,255,255),"Survivor_upgrades enable!\n");
}

CON_COMMAND(l4d_sv_gravity, "sv_gravity.\n"){

	l4d_sv_gravity_click++;

	if(l4d_sv_gravity_click==1){
		ConVar * su = NULL;
		su = g_pCVar->FindVar("sv_gravity");
		if(su == NULL){
			Warning("Failed to find ConVar sv_gravity!\n");
			return;
		}
		su->RemoveFlags(su->GetFlags());
		engine_sv->ServerCommand("sv_gravity 400\n");
		engine_sv->Con_NXPrintf(&HudMsg(R,G,B,T),"Sv_gravity 400\n");
	}
	if(l4d_sv_gravity_click>1){
		engine_sv->ServerCommand("sv_gravity 800\n");
		l4d_sv_gravity_click=0;
		engine_sv->Con_NXPrintf(&HudMsg(R,G,B,T),"Sv_gravity 800\n");
	}
}

CON_COMMAND(l4d_developer, "developer.\n"){
	
	l4d_developer_click++;

	if(l4d_developer_click==1){
		ConVar * dev = NULL;
		dev = g_pCVar->FindVar("developer");
		if(dev == NULL){
			Warning("Failed to find ConVar developer!\n");
			return;
		}
		dev->RemoveFlags(dev->GetFlags());
		engine_sv->ServerCommand("developer 1\n");
		engine_sv->Con_NXPrintf(&HudMsg(R,G,B,T),"Developer on\n");
	}
	if(l4d_developer_click>1){
		engine_sv->ServerCommand("developer 0\n");
		engine_sv->Con_NXPrintf(&HudMsg(R,G,B,T),"Developer off\n");
		l4d_developer_click=0;
	}
}

CON_COMMAND(l4d_sv_friction, "sv_friction.\n"){
	
	l4d_sv_friction_click++;

	if(l4d_sv_friction_click==1){
		ConVar * svf = NULL;
		svf = g_pCVar->FindVar("sv_friction");
		if(svf == NULL){
			Warning("Failed to find ConVar sv_friction!\n");
			return;
		}
		svf->RemoveFlags(svf->GetFlags());
		engine_sv->ServerCommand("sv_friction 0\n");
		engine_sv->Con_NXPrintf(&HudMsg(R,G,B,T),"Sv_friction 0\n");
	}
	if(l4d_sv_friction_click>1){
		engine_sv->ServerCommand("sv_friction 4\n");
		engine_sv->Con_NXPrintf(&HudMsg(R,G,B,T),"sv_friction 4\n");
		l4d_sv_friction_click=0;
	}
}

CON_COMMAND(l4d_vcollide_wireframe, "vcollide_wireframe.\n"){

	l4d_vcollide_wireframe_click++;
	if(l4d_vcollide_wireframe_click==1){
		ConVar * vcw = NULL;

		vcw = g_pCVar->FindVar("vcollide_wireframe");
		if(vcw == NULL){
			Warning("Failed to find Cvar vcollide_wireframe!\n");
			return;
		}

		vcw->RemoveFlags(vcw->GetFlags());
		engine_sv->ServerCommand("vcollide_wireframe 1\n");
	}
	if(l4d_vcollide_wireframe_click>1){
		engine_sv->ServerCommand("vcollide_wireframe 0\n");
		l4d_vcollide_wireframe_click = 0;
	}
}

CON_COMMAND(l4d_versus_tool_write_autoexec, "l4d_write_autoexec.\n"){
	char cfg[256];
	engine_sv->GetGameDir(cfg,sizeof(cfg));
	strcat(cfg,"/cfg/autoexec.cfg");
	if(FileExists(cfg)){
		FILE *CfgFile;
		CfgFile=fopen(cfg,"a+");
		fprintf(CfgFile,"\n\n//++=====L4D Versus Tool=====++\n\n");

		fprintf(CfgFile,"//----Crosshair----\n");
		fprintf(CfgFile,"cl_crosshair_red 0\n");
		fprintf(CfgFile,"cl_crosshair_blue 0\n");
		fprintf(CfgFile,"cl_crosshair_green 255\n\n");

		fprintf(CfgFile,"//----Game_Instructor----\n");
		fprintf(CfgFile,"gameinstructor_enable 0\n\n");

		fprintf(CfgFile,"//----Mouse_Sensitivity----\n");
		fprintf(CfgFile,"sensitivity 4.8\n\n");

		fprintf(CfgFile,"//----Film_grain----\n");
		fprintf(CfgFile,"mat_grain_scale_override 0\n\n");

		fprintf(CfgFile,"//----Key_Binds----\n");
		fprintf(CfgFile,"bind q +zoom\n");
		fprintf(CfgFile,"bind SHIFT l4d_thirdperson\n");
		fprintf(CfgFile,"bind ALT l4d_r_drawothermodels\n");
		fprintf(CfgFile,"bind RSHIFT l4d_host_timescale\n");
		fprintf(CfgFile,"bind RSHIFT l4d_host_timescale\n");
		fprintf(CfgFile,"bind BACKSPACE l4d_record_demo\n");
		fprintf(CfgFile,"bind n \"incrementvar net_graph 0 4 4\"\n");
		fprintf(CfgFile,"bind v noclip\n");
		fprintf(CfgFile,"bind f4 l4d_slow_motion\n");
		fprintf(CfgFile,"bind f3 openserverbrowser\n");
		fprintf(CfgFile,"bind [ \"say_team !teams\"\n");
		fprintf(CfgFile,"bind l \"say_team !laser\"\n");
		fprintf(CfgFile,"bind g \"say_team !buy\"\n\n");

		fprintf(CfgFile,"//----Versus_Glow_settings---\n");
		fprintf(CfgFile,"cl_glow_survivor_health_high_r 0\n");
		fprintf(CfgFile,"cl_glow_survivor_health_high_g 255\n");
		fprintf(CfgFile,"cl_glow_survivor_health_high_b 0\n\n");

		fprintf(CfgFile,"cl_glow_survivor_health_med_r 255\n");
		fprintf(CfgFile,"cl_glow_survivor_health_med_g 255\n");
		fprintf(CfgFile,"cl_glow_survivor_health_med_b 0\n\n");

		fprintf(CfgFile,"cl_glow_survivor_health_low_r 255\n");
		fprintf(CfgFile,"cl_glow_survivor_health_low_g 0\n");
		fprintf(CfgFile,"cl_glow_survivor_health_low_b 0\n\n");
		 
		fclose(CfgFile);
		ConColorMsg(Color(0,255,0,255),"Config was created!\n");
		engine_sv->ServerCommand("exec autoexec\n");
		ConColorMsg(Color(0,255,0,255),"Config was applied!\n");
	}
	else{
		FILE *CfgFile;
		CfgFile=fopen(cfg,"w");
		
		fprintf(CfgFile,"//++=====L4D Versus Tool=====++\n\n");

		fprintf(CfgFile,"//----Crosshair----\n");
		fprintf(CfgFile,"cl_crosshair_red 0\n");
		fprintf(CfgFile,"cl_crosshair_blue 0\n");
		fprintf(CfgFile,"cl_crosshair_green 255\n\n");

		fprintf(CfgFile,"//----Game_Instructor----\n");
		fprintf(CfgFile,"gameinstructor_enable 0\n\n");

		fprintf(CfgFile,"//----Mouse_Sensitivity----\n");
		fprintf(CfgFile,"sensitivity 4.8\n\n");

		fprintf(CfgFile,"//----Film_grain----\n");
		fprintf(CfgFile,"mat_grain_scale_override 0\n\n");

		fprintf(CfgFile,"//----Key_Binds----\n");
		fprintf(CfgFile,"bind q +zoom\n");
		fprintf(CfgFile,"bind SHIFT l4d_thirdperson\n");
		fprintf(CfgFile,"bind ALT l4d_r_drawothermodels\n");
		fprintf(CfgFile,"bind RSHIFT l4d_host_timescale\n");
		fprintf(CfgFile,"bind RSHIFT l4d_host_timescale\n");
		fprintf(CfgFile,"bind BACKSPACE l4d_record_demo\n");
		fprintf(CfgFile,"bind n \"incrementvar net_graph 0 4 4\"\n");
		fprintf(CfgFile,"bind v noclip\n");
		fprintf(CfgFile,"bind f4 l4d_slow_motion\n");
		fprintf(CfgFile,"bind f3 openserverbrowser\n");
		fprintf(CfgFile,"bind [ \"say_team !teams\"\n");
		fprintf(CfgFile,"bind l \"say_team !laser\"\n");
		fprintf(CfgFile,"bind g \"say_team !buy\"\n\n");

		fprintf(CfgFile,"//----Versus_Glow_settings---\n");
		fprintf(CfgFile,"cl_glow_survivor_health_high_r 0\n");
		fprintf(CfgFile,"cl_glow_survivor_health_high_g 255\n");
		fprintf(CfgFile,"cl_glow_survivor_health_high_b 0\n\n");

		fprintf(CfgFile,"cl_glow_survivor_health_med_r 255\n");
		fprintf(CfgFile,"cl_glow_survivor_health_med_g 255\n");
		fprintf(CfgFile,"cl_glow_survivor_health_med_b 0\n\n");

		fprintf(CfgFile,"cl_glow_survivor_health_low_r 255\n");
		fprintf(CfgFile,"cl_glow_survivor_health_low_g 0\n");
		fprintf(CfgFile,"cl_glow_survivor_health_low_b 0\n\n");

		fclose(CfgFile);
		ConColorMsg(Color(0,255,0,255),"Config was created!\n");
		engine_sv->ServerCommand("exec autoexec\n");
		ConColorMsg(Color(0,255,0,255),"Config was applied!\n");
	}	
}

CON_COMMAND(l4d_slow_motion, "l4d_slow_motion.\n"){

	l4d_slow_motion_click++;

	if(l4d_slow_motion_click==1){
		ConVar *svc = NULL;
		ConVar * hts = NULL;

		svc = g_pCVar->FindVar("sv_cheats");
		if(svc == NULL){
			Warning("Failed to find ConVar sv_cheats!\n");
			return;
		}
		svc->RemoveFlags(svc->GetFlags());
		engine_sv->ServerCommand("sv_cheats 1\n");

		hts = g_pCVar->FindVar("host_timescale");
		if(hts == NULL){
			Warning("Failed to find ConVar host_timescale!\n");
			return;
		}
		hts->RemoveFlags(hts->GetFlags());
		engine_sv->ServerCommand("host_timescale 0.1\n");
		engine_sv->Con_NXPrintf(&HudMsg(R,G,B,T),"Slow_motion on\n");
	}
	
	if(l4d_slow_motion_click>1){
		engine_sv->ServerCommand("host_timescale 1.0\n");
		engine_sv->ServerCommand("sv_cheats 0\n");
		engine_sv->Con_NXPrintf(&HudMsg(R,G,B,T),"Slow_motion off\n");
		l4d_slow_motion_click=0;
	}
}

CON_COMMAND(l4d_versus_tool_create_versus_config, "l4d_versus_tool_create_versus_config.\n"){

	char cfg_versus[256];
	engine_sv->GetGameDir(cfg_versus,sizeof(cfg_versus));
	strcat(cfg_versus,"/cfg/versus.cfg");

	if(FileExists(cfg_versus)){
		ConColorMsg(Color(0,255,0,255),"Config exist!\n");
	}
	else{
		FILE *CfgVersus;
		CfgVersus=fopen(cfg_versus,"w");
		fprintf(CfgVersus,"//++=====L4D Versus Training=====++\n\n");

		fprintf(CfgVersus,"sv_cheats 1\n");
		fprintf(CfgVersus,"endround\n");
		fprintf(CfgVersus,"mp_gamemode versus\n");
		fprintf(CfgVersus,"vs_max_team_switches 50\n");
		fprintf(CfgVersus,"chooseteam\n");
		fprintf(CfgVersus,"sb_all_bot_team 1\n");
		fprintf(CfgVersus,"clear\n");
		fprintf(CfgVersus,"echo Versus mode Enable!\n");
		fclose(CfgVersus);
		ConColorMsg(Color(0,255,0,255),"L4D Versus Training config created!\n");
	}
}

CON_COMMAND(l4d_duck_toggle, "l4d_duck_toggle.\n"){
	if(atoi(args[1])==1||atoi(args[1])>1){
		engine_sv->ServerCommand("+duck\n");
	}
	if(atoi(args[1])==0||atoi(args[1])<0){
		engine_sv->ServerCommand("-duck\n");
	}
}

CON_COMMAND(l4d_reload_toggle, "l4d_reload_toggle.\n"){
	if(atoi(args[1])==1||atoi(args[1])>1){
		engine_sv->ServerCommand("+reload\n");
	}
	if(atoi(args[1])==0||atoi(args[1])<0){
		engine_sv->ServerCommand("-reload\n");
	}
}

CON_COMMAND(l4d_print_server_entitys,"l4d_print_server_entitys"){
	if(ienginetool->IsInGame()&&SP()){
		if(strlen(args.Arg(1))>0){
			for(int i=0;i<globals->maxEntities;i++){
				if(!strcmp(PEntityOfEntIndex(i)->GetClassNameA(),args.Arg(1))){
					Msg("# %d\n",i);
					Msg("%s\n",PEntityOfEntIndex(i)->GetClassNameA());
					Msg("BaseEnt: %x\n\n",sv_ents->EdictToBaseEntity(PEntityOfEntIndex(i)));
				}
			}
		}
		else{
			for(int i=0;i<globals->maxEntities;i++){
				Msg("# %d\n",i);
				Msg("%s\n",PEntityOfEntIndex(i)->GetClassNameA());
				Msg("BaseEnt: %x\n\n",sv_ents->EdictToBaseEntity(PEntityOfEntIndex(i)));
			}
		}
	}
}

CON_COMMAND(l4d_remove_edict,"l4d_remove_edict"){
	if(args.ArgC()>1&&ienginetool->IsInGame()&&SP()){
		engine_sv->RemoveEdict(PEntityOfEntIndex(atoi(args.ArgS())));
	}
}

CON_COMMAND(l4d_remove_edicts_by_name,"l4d_remove_edicts_by_name"){
	if(args.ArgC()>1&&ienginetool->IsInGame()&&SP()){
		int edict_count=0;
		for(int i=0;i<globals->maxEntities;i++){
			if(!strcmp(args.ArgS(),PEntityOfEntIndex(i)->GetClassNameA())){
				engine_sv->RemoveEdict(PEntityOfEntIndex(i));
				edict_count++;
			}
		}
		if(edict_count>0){
			Msg("Removed %d %s(s)\n",edict_count,args.ArgS());
		}
		else{
			Msg("Nothing to remove\n");
		}
	}
}

CON_COMMAND(l4d_print_client_entitys, "l4d_print_client_entitys.\n"){
	
	if(ienginetool->IsInGame()){
		int ents = entList->GetMaxEntities();
		IClientEntity *cl_ent;
		C_BaseEntity *pEntity = NULL;

		for (int i=0; i<ents; i++){
			cl_ent = entList->GetClientEntity(i);
			if(cl_ent!=null){
				if(strlen(args.Arg(1))>0){
					if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,args.Arg(1))){
						Msg("# %d Class: %s\n",i,cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName);
						pEntity = cl_ent->GetBaseEntity();
						Msg("BaseEnt: %x\n",pEntity);
						}
					}
				else{
				Msg("# %d Class: %s\n",i,cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName);
				pEntity = cl_ent->GetBaseEntity();
				Msg("BaseEnt: %x\n\n",pEntity);
				}
			}
		}
	}
}

//Remove :private specificator in dt_recv.h for work this.
/*void DumpClass( ClientClass *pClass, RecvTable *pTable, int tab ){
	RecvProp *pProp = NULL;

	if ( pTable->m_nProps < 0 )
		return;

	char pad[1024] = "";

	for( int j = 0; j < tab; j++ )
		strcat( pad, "\t" );

	Msg( "%s%s\n", pad, pTable->m_pNetTableName );

	for( int i = 0; i < pTable->m_nProps; i++ )
	{		
		pProp = &pTable->m_pProps[i];

		if( !pProp )
			continue;		

		char szHashBuf[256];
		sprintf( szHashBuf, "%s%s", pTable->m_pNetTableName, pProp->m_pVarName );

		DWORD_PTR dwHash = CRC32( ( void* )szHashBuf, strlen ( szHashBuf ) );

		Msg( "%s%s 0x%X [0x%X]\n", pad, pProp->m_pVarName, pProp->m_Offset, dwHash );

		if ( pProp->m_pVarName[0] == 0x30 )
		{
			if ( pProp->m_pDataTable )
				DumpClass( pClass, pProp->m_pDataTable, tab+1 );
			else
				continue;
		}

		if ( pProp->m_pDataTable )
			DumpClass( pClass, pProp->m_pDataTable, tab+1 );
		else
			continue;
	}
}*/

void DumpClass_Client( ClientClass *pClass, RecvTable *pTable ){
	RecvProp *pProp = NULL;

	if ( pTable->m_nProps < 0 )
		return;

	ConColorMsg(Color(0,255,0,255),"%s\n", pTable->m_pNetTableName );

	for( int i = 0; i < pTable->m_nProps; i++ )
	{		
		pProp = &pTable->m_pProps[i];

		if( !pProp )
			continue;		

		char szHashBuf[256];
		sprintf( szHashBuf, "%s%s", pTable->m_pNetTableName, pProp->m_pVarName );

		ConColorMsg(Color(255,216,0,255),"	%s 0x%X\n", pProp->m_pVarName, pProp->m_Offset );

		if ( pProp->m_pVarName[0] == 0x30 )
		{
			if ( pProp->m_pDataTable )
				DumpClass_Client( pClass, pProp->m_pDataTable );
			else
				continue;
		}

		if ( pProp->m_pDataTable )
			DumpClass_Client( pClass, pProp->m_pDataTable );
		else
			continue;
	}
}

void DumpClass_Server( ServerClass *pClass, SendTable *pTable ){
	SendProp *pProp = NULL;

	if ( pTable->m_nProps < 0 )
		return;

	ConColorMsg(Color(0,255,0,255),"%s\n", pTable->m_pNetTableName );

	for( int i = 0; i < pTable->m_nProps; i++ ){
		pProp = &pTable->m_pProps[i];

		if( !pProp )
			continue;	

		char szHashBuf[256];
		sprintf( szHashBuf, "%s%s", pTable->m_pNetTableName, pProp->m_pVarName );

		ConColorMsg(Color(255,216,0,255),"	%s 0x%X\n", pProp->m_pVarName, pProp->m_Offset );

		if ( pProp->m_pVarName[0] == 0x30 )
		{
			if ( pProp->m_pDataTable )
				DumpClass_Server( pClass, pProp->m_pDataTable );
			else
				continue;
		}

		if ( pProp->m_pDataTable )
			DumpClass_Server( pClass, pProp->m_pDataTable );
		else
			continue;
	}
}

CON_COMMAND(l4d_print_client_class_recvtable, "Print client m_pRecvTable.\n"){

	if(ienginetool->IsInGame()&&strlen(args.Arg(1))!=0){
		RecvTable *pTable = NULL;
		ClientClass *cl_class = NULL;
		int max_ents = entList->GetMaxEntities();
		IClientEntity *cl_ent;

		for (int i=0; i<max_ents; i++){
			cl_ent = entList->GetClientEntity(i);
			if(cl_ent!=null){
				if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,args.Arg(1))){
					cl_class = cl_ent->GetBaseEntity()->GetClientClass();
					if(cl_class) pTable = cl_class->m_pRecvTable;
					break;
				}
			}
		}
		if(pTable==NULL){
			Warning("Client entity not Found!\n");
			return;
		}
		DumpClass_Client(cl_class,pTable);
	}
}


CON_COMMAND(l4d_print_server_class_sendtable, "Print server entity SendTable.\n"){
	if(ienginetool->IsInGame()&&strlen(args.Arg(1))!=0){
		SendTable *pTable = NULL;
		ServerClass *sv_class = NULL;

		for(int i=0;i<globals->maxEntities;i++){
			if(!strcmp(PEntityOfEntIndex(i)->GetClassNameA(),args.Arg(1))){
				sv_class = PEntityOfEntIndex(i)->GetIServerEntity()->GetNetworkable()->GetServerClass();
				if(sv_class) pTable = sv_class->m_pTable;
				break;
			}
		}
		if(pTable==NULL){
			Warning("Server entity not found!\n");
			return;
		}
		DumpClass_Server(sv_class,pTable);
	}
}

CON_COMMAND(l4d_get_server_classes, "Dumps the server class list as a text file."){

	remove("L4D_Server_Classes.txt");
	FILE *fp = NULL;
	int count = 0;
	fp=fopen("L4D_Server_Classes.txt","w");
	ServerClass *pBase = gamedll->GetAllServerClasses();
	fprintf(fp, "// Dump of all classes for \"Left 4 Dead\"\n\n");
	while (pBase != NULL)
	{
		count++;
		fprintf(fp, "%d %s\n%x\n\n",count, pBase->GetName(), pBase);
		Msg("#%d %s\n%x\n\n",count, pBase->GetName(), pBase);
		if(args.ArgC()>0){
			if(!strcmp(pBase->GetName(),args.ArgS())){
				DumpClass_Server(pBase,pBase->m_pTable);
				return;
			}
		}
		pBase = pBase->m_pNext;
	}
	fclose(fp);
	ConColorMsg(Color(0,255,0,255),"Class list has been saved to 'L4D_Classes.txt' in your game folder.\n");
}

CON_COMMAND(l4d_zombie_kill_stats, "Show killed zombies stats.\n"){
	if(KillCounter.GetInt()>0){
		ConColorMsg(Color(0,255,0,255),"Killed zombies: %d - (Headshot: %d) - Killed players: %d\n",zombies,headshots,players);
		engine_sv->Con_NXPrintf(&HudMsg(0.9,0.9,0.9,5.0),"Killed zombies: %d - (Headshot: %d) - Killed players: %d",zombies,headshots,players);
	}
}
CON_COMMAND(l4d_spawn_entity_by_name, "Spawn entity by name.\n"){
	CBaseEntity *pEntity = (CBaseEntity *)server_tool->CreateEntityByName(args.Arg(1));
	server_tool->SetKeyValue(pEntity,"orign","0 0 0");
	server_tool->DispatchSpawn(pEntity);
	Msg("%x\n",pEntity);
}

CON_COMMAND(l4d_main_thread_terminate, "Terminate main thread.\n"){
	isThreadActive = false;
	Msg("Main Thread terminated!\n");
}

CON_COMMAND(l4d_main_thread_start, "Run main thread.\n"){
	isThreadActive = true;
	l4d_thread = CreateThread(NULL,NULL,L4D_VT_MainThread,NULL,NULL,NULL);
	Msg("Main Thread is runing!\n");
}

CON_COMMAND(l4d_mat_wallhack, "l4d_mat_wallhack.\n"){
	/*Old Method
	if(materials->IsTextureLoaded("models\\infected\\hunter\\hunter_01")){
		materials->FindMaterial("models\\infected\\hunter\\hunter_01", "Model textures")->SetMaterialVarFlag( MATERIAL_VAR_IGNOREZ, state);
	}*/
	int max_entitys = entList->GetMaxEntities();
	IClientEntity *cl_ent;
	C_BaseEntity *PZBot = NULL;
	IMaterial *PZBot_mats[MAXSTUDIOSKINS];

	for(int i=0; i<max_entitys; i++){
		cl_ent = entList->GetClientEntity(i);
		if(cl_ent!=null){
			if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Smoker")||
			!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Boomer")||
			!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Hunter")||
			!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"Tank")){
				PZBot = cl_ent->GetBaseEntity();
				const model_t *PZBot_model = GetEntityModel(PZBot);
				studiohdr_t* PZBot_hdr = model_info->GetStudiomodel(PZBot_model);
				model_info->GetModelMaterials(PZBot_model,PZBot_hdr->numtextures,PZBot_mats);

				for (int i = 0; i < PZBot_hdr->numtextures; i++){
					IMaterial* PZBot_mat = PZBot_mats[i];
					if (!PZBot_mat) continue;
					PZBot_mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
					/*IMaterialVar *mvar = PZBot_mat->FindVar("$basetexture",NULL,false);
					IMaterial *white = materials->FindMaterial("models\\infected\\smoker\\smoker", "Model textures");
					mvar->SetMaterialValue(white);
					PZBot_mat->Refresh();*/
				}
			}
		}
	}

	C_BaseEntity *PZPlayer = NULL;
	IMaterial *PZPlayer_mats[MAXSTUDIOSKINS];

	for(int i=0; i<max_entitys; i++){
		cl_ent = entList->GetClientEntity(i);
		if(cl_ent!=null){
			if(!strcmp(cl_ent->GetBaseEntity()->GetClientClass()->m_pNetworkName,"CTerrorPlayer")){
				PZPlayer = cl_ent->GetBaseEntity();
				int *m_iTeamNum = (int*) ( ( DWORD )PZPlayer + ( DWORD )0xBC );
				if(*m_iTeamNum==Infected){
					PZBot = cl_ent->GetBaseEntity();
					const model_t *PZPlayer_model = GetEntityModel(PZPlayer);
					studiohdr_t* PZPlayer_hdr = model_info->GetStudiomodel(PZPlayer_model);
					model_info->GetModelMaterials(PZPlayer_model,PZPlayer_hdr->numtextures,PZPlayer_mats);

					for (int i = 0; i < PZPlayer_hdr->numtextures; i++){
						IMaterial* PZPlayer_mat = PZPlayer_mats[i];
						if (!PZPlayer_mat) continue;
						PZPlayer_mat->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, true);
					}
				}
			}
		}
	}
}

CON_COMMAND(l4d_get_owner_aim_ent_id, "l4d_get_owner_aim_ent_id.\n"){
	if(!ienginetool->IsInGame()){
		Warning("It's system command!\n");
		return;
	}
	CBaseEntity *owner = NULL;
	Vector eye_position, aim_dir, vec_end;
	QAngle eye_angles;

	eye_position.Init();
	aim_dir.Init();
	vec_end.Init();
	eye_angles.Init();

	owner = GetOwnerEntity();
	eye_position = GetOwnerPos();
	engine_cl->GetViewAngles(eye_angles);
	AngleVectors(eye_angles, &aim_dir);
	VectorNormalize(aim_dir);
	vec_end = eye_position + aim_dir * 80000;
	if(L4DDrawRay.GetBool()) debug->AddLineOverlay(GetOwnerPos(),vec_end,255,0,0,true,0);

	Ray_t  ray;
	ray.Init(GetOwnerPos()+Vector(0,0,75),vec_end);
	trace_t tr;
			
	enginetrace->TraceRay(ray,MASK_NPCWORLDSTATIC|MASK_VISIBLE_AND_NPCS|MASK_SHOT,NULL,&tr);	

	EntID = tr.m_pEnt->entindex();
	//Msg("%d\n",EntID);
}

CON_COMMAND(l4d_ent_is_visible, "l4d_ent_is_visible.\n"){
	if(!ienginetool->IsInGame()){
		Warning("It's system command!\n");
		return;
	}
	if(L4DDrawRay.GetBool()) debug->AddLineOverlay(GetOwnerPos(),EnemyBone_v,255,0,0,true,0);
	Ray_t  ray;
	ray.Init(GetOwnerPos()+Vector(0,0,75),EnemyBone_v);
	trace_t tr;
	enginetrace->TraceRay(ray,MASK_NPCWORLDSTATIC|MASK_VISIBLE_AND_NPCS|MASK_SHOT,NULL,&tr);	
	MobID = tr.m_pEnt->entindex();
}

CON_COMMAND(l4d_get_owner_aim_pos, "l4d_get_owner_aim_pos.\n"){
	if(!ienginetool->IsInGame()){
		Warning("It's system command!\n");
		return;
	}
	CBaseEntity *owner = NULL;
	Vector eye_position, aim_dir, vec_end;
	QAngle eye_angles;

	eye_position.Init();
	aim_dir.Init();
	vec_end.Init();
	eye_angles.Init();

	owner = GetOwnerEntity();
	eye_position = GetOwnerPos();
	engine_cl->GetViewAngles(eye_angles);
	AngleVectors(eye_angles, &aim_dir);
	VectorNormalize(aim_dir);
	vec_end = eye_position + aim_dir * 8000;

	Ray_t  ray;
	ray.Init(GetOwnerPos()+Vector(0,0,75),vec_end);
	trace_t tr;

	enginetrace->TraceRay(ray,MASK_SOLID,NULL,&tr);
	OwnerAimPos = tr.endpos;
	//Msg("%f %f %f\n",tr.endpos.x,tr.endpos.y,tr.endpos.z);

}

void MatrixGetColumn2( const matrix3x4_t& in, int column, Vector &out )
{
	out.x = in[0][column];
	out.y = in[1][column];
	out.z = in[2][column];
}

void PrintMatrix(matrix3x4_t matrix){
	int j,k;
	for(j=0;j<3;j++){
		for(k=0;k<4;k++){
			Msg("matrix[%d][%d] = %f\n",j,k,matrix[j][k]);
		}
		Msg("|----|\n");
	}
}

void PrintMatrix(VMatrix matrix){
	int j,k;
	for(j=0;j<4;j++){
		for(k=0;k<4;k++){
			Msg("matrix[%d][%d] = %f\n",j,k,matrix[j][k]);
		}
		Msg("|----|\n");
	}
}

CON_COMMAND(l4d_setup_bones, "l4d_setup_bones.\n")
{
	if(!ienginetool->IsInGame()){
		Warning("It's system command!\n");
		return;
	}
	BonePos.Init();
	BoneAng.Init();
	
	IClientEntity *cl_ent = entList->GetClientEntity(ClientNum);
	IClientRenderable* RenderEntity = cl_ent->GetClientRenderable();
	if(RenderEntity->SetupBones( bone_matrix, 128, BONE_USED_BY_HITBOX, 0))
		MatrixAngles( bone_matrix[ BoneNum ], BoneAng, BonePos );
	/*Set this:
	Vector BonePos;
	QAngle BoneAng;
	ClientNum = 1;
	BoneNum = 0;
	engine_sv->ServerCommand("l4d_setup_bones\n");
	*/
}

void drawBox (Vector const *v, float const * color )
{
	CMatRenderContextPtr pRenderContext( materials );
	IMesh* pMesh = pRenderContext->GetDynamicMesh( );

	CMeshBuilder meshBuilder;

	// The four sides
	meshBuilder.Begin( pMesh, MATERIAL_TRIANGLE_STRIP, 2 * 4 );
	for (int i = 0; i < 10; i++)
	{
		meshBuilder.Position3fv (v[i & 7].Base() );
		meshBuilder.Color4fv( color );
		meshBuilder.AdvanceVertex();
	}
	meshBuilder.End();
	pMesh->Draw();

	// top and bottom
	meshBuilder.Begin( pMesh, MATERIAL_TRIANGLE_STRIP, 2 );

	meshBuilder.Position3fv (v[6].Base());
	meshBuilder.Color4fv( color );
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3fv (v[0].Base());
	meshBuilder.Color4fv( color );
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3fv (v[4].Base());
	meshBuilder.Color4fv( color );
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3fv (v[2].Base());
	meshBuilder.Color4fv( color );
	meshBuilder.AdvanceVertex();

	meshBuilder.End();
	pMesh->Draw();

	meshBuilder.Begin( pMesh, MATERIAL_TRIANGLE_STRIP, 2 );

	meshBuilder.Position3fv (v[1].Base());
	meshBuilder.Color4fv( color );
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3fv (v[7].Base());
	meshBuilder.Color4fv( color );
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3fv (v[3].Base());
	meshBuilder.Color4fv( color );
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3fv (v[5].Base());
	meshBuilder.Color4fv( color );
	meshBuilder.AdvanceVertex();

	meshBuilder.End();
	pMesh->Draw();
}

void TestMsg(){
	/*//Call Msg func from tier0.dll with offset, IDA helper 
	typedef void (* FuncType)(const tchar* pMsg, ...);
	FuncType FunctionAddress;
	HINSTANCE hint = LoadLibrary("bin/tier0.dll");
	FunctionAddress = (FuncType)GetProcAddress(hint,"Msg");
	Msg("%x\n",FunctionAddress);
	if(FunctionAddress){
			FunctionAddress("Test\n");
		}*/
	
	/*//Call Msg func from tier0.dll with offset
	typedef void (* FuncType)(const tchar* pMsg, ...);
	FuncType FunctionAddress;
	FunctionAddress = (FuncType)(( DWORD )GetModuleHandleA("tier0.dll")+( DWORD )0x4290);
	if(FunctionAddress){
			FunctionAddress("Test\n");
		}
	*/

	#define SIG "\xB8\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8B\x8C\x24\x00\x00\x00\x00\x8D\x84\x24\x00\x00\x00\x00\x50\x51\x8D\x54\x24\x14\x68\x00\x00\x00\x00\x52\xE8\x00\x00\x00\x00\x83\xC4\x10\x83\xF8\xFF\x74\x68\x56\x8D\x44\x24\x04\x50\xB9\x00\x00\x00\x00\xC7\x44\x24\x00\x00\x00\x00\x00\xC7\x44\x24\x00\x00\x00\x00\x00\xC7\x44\x24\x00\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8D\x4C\x24\x10\x51\x6A\x00\xFF\x15\x00\x00\x00\x00"
	#define MASK "x????x????xxx????xxx????xxxxxxx????xx????xxxxxxxxxxxxxxx????xxx?????xxx?????xxx?????x????xxxxxxxxx????"
	typedef void (* FuncType)(const tchar* pMsg, ...);
	FuncType FunctionAddress;
	FunctionAddress = (FuncType)FindPattern("tier0.dll",SIG,MASK);
	if(FunctionAddress){
		FunctionAddress("Test msg\n");
	}
}

CON_COMMAND(l4d_set_crosshair_material_transparent, "Make the crosshair material is transparent.\n"){
	if(client->IsInGame()){
		IMaterial *crosshair_material = NULL;
		typedef IMaterial *(*FuncType)();
		FuncType GetMaterialAtCrossHair;
		GetMaterialAtCrossHair = (FuncType)FindPattern("engine.dll",GetMaterialAtCrossHair_SIG,GetMaterialAtCrossHair_MASK);
		Msg("%x\n",GetMaterialAtCrossHair);
		if(GetMaterialAtCrossHair) crosshair_material = GetMaterialAtCrossHair();
		if(crosshair_material){
			if(materials->IsTextureLoaded(crosshair_material->GetName())){
				Msg("%s\n",crosshair_material->GetName());
				engine_sv->ServerCommand("playgamesound UI\\menu_click04.wav\n");
				if(!crosshair_material->GetMaterialVarFlag(MATERIAL_VAR_NO_DRAW)){
					crosshair_material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, 1);
					engine_sv->Con_NXPrintf(&HudMsg(0,0.9,0,T),"materal_visible: %s - off",crosshair_material->GetName());
					return;
				}
				if(crosshair_material->GetMaterialVarFlag(MATERIAL_VAR_NO_DRAW)){
					crosshair_material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, 0);
					engine_sv->Con_NXPrintf(&HudMsg(0,0.9,0,T),"materal_visible: %s - on",crosshair_material->GetName());
					return;
				}
			}
		}
	}
	else 
		Warning("Run gameplay!\n");
}

CON_COMMAND(l4d_enable_skeleton_draw, "l4d_enable_skeleton_draw.\n")
{
	ConVar *sk = g_pCVar->FindVar("enable_skeleton_draw");
	if(sk == NULL){
		Warning("Failed to find ConVar enable_skeleton_draw!\n");
		return;
	}
	sk->RemoveFlags(sk->GetFlags());
	engine_sv->ServerCommand("enable_skeleton_draw 1\n");
}


CON_COMMAND(l4d_test, "l4d_test.\n")
{
}

