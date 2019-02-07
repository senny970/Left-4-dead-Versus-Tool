#pragma once

#include "icliententity.h"

const DWORD Offset_Entity_Pos_X = 0x06C;
const DWORD Offset_Entity_Pos_Y = 0x070;
const DWORD Offset_Entity_Pos_Z = 0x074;

const DWORD Offset_Entity_Angles_P = 0x090;
const DWORD Offset_Entity_Angles_Y = 0x094;
const DWORD Offset_Entity_Angles_R = 0x098;

const DWORD Offset_Entity_Class  = 0x1964;

const DWORD Offset_Entity_Team = 0x0bc;

const DWORD Offset_Entity_Health = 0x0c4;

const DWORD Offset_Entity_Free = 0x138;


//---------------------------------------------------------------------------------
// Purpose: 
//--------------------------------------------------------------------------------
void SetDataFromByteOffset(void* ptrClass, int byteOffset, BYTE data) 
{ 
	if (byteOffset <= 0) 
		return; 
	*(reinterpret_cast<BYTE*>((char*)ptrClass + byteOffset)) = data; 
}
//--------------------------------------------------------------------------------
void SetDataFromByteOffset(void* ptrClass, int byteOffset, int data) 
{ 
	if (byteOffset <= 0) 
		return; 
	*(reinterpret_cast<int*>((char*)ptrClass + byteOffset)) = data; 
}
//--------------------------------------------------------------------------------
void SetDataFromByteOffset(void* ptrClass, int byteOffset, float data) 
{ 
	if (byteOffset <= 0) 
		return; 
	*(reinterpret_cast<float*>((char*)ptrClass + byteOffset)) = data; 
}
//---------------------------------------------------------------------------------
// Purpose: a sample 3rd party plugin class
//--------------------------------------------------------------------------------
class IBaseClientDLL_fix;
template< typename Function > Function call_vfunc( PVOID Base, DWORD Index )
{
	PDWORD* VTablePointer = ( PDWORD* )Base;
	PDWORD VTableFunctionBase = *VTablePointer;
	DWORD dwAddress = VTableFunctionBase[ Index ];
	return ( Function )( dwAddress );
}

//---------------------------------------------------------------------------------
// Purpose: 
//--------------------------------------------------------------------------------
typedef void* (*CreateInterfaceFn)(const char *pName, int *pReturnCode);
CreateInterfaceFn CaptureFactory(char *pszFactoryModule)
{
	CreateInterfaceFn fn = NULL;
	while ( fn == NULL)
	{
		HMODULE hFactoryModule = GetModuleHandleA(pszFactoryModule);
		if (hFactoryModule)
		{
			fn = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(hFactoryModule, "CreateInterface"));
		}
		Sleep(10);
	}
	return fn;
}
//---------------------------------------------------------------------------------
// Purpose: 
//--------------------------------------------------------------------------------
class IBaseClientDLL_fix
{
public:
	ClientClass *GetAllClasses(){
		typedef ClientClass*(__thiscall* oGetClientClass)( PVOID);
		return call_vfunc< oGetClientClass>(this,7)(this);
	}
	bool IN_IsKeyDown(const char * name,  bool& isdown ){
		typedef bool(__thiscall* oIN_IsKeyDown)( PVOID,const char *, bool&);
		return call_vfunc< oIN_IsKeyDown>(this,7)(this, name, isdown);
	}
	//void CreateMove( int sequence_number, float frametime, CUserCmd active )
	//{
	//	typedef void ( __thiscall* oMove )( PVOID,int,float,CUserCmd );
	//	return call_vfunc< oMove >( this, 20 )( this, sequence_number, frametime, active );
	//}

		//vgui::VPANEL GetFullscreenClientDLLVPanel(){
		//typedef vgui::VPANEL(__thiscall* oGetFullscreenClientDLLVPanel)( PVOID);
		//return call_vfunc< oGetFullscreenClientDLLVPanel>(this,79)(this);
	//}
};

//---------------------------------------------------------------------------------
// Purpose: 
//--------------------------------------------------------------------------------
class C_BaseEntity : public IClientEntity{//, public IClientUnknown, public IClientRenderable, public IClientNetworkable, public IClientThinkable{ // "DT_BasePlayer"

public:	
	const int GetPl() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x010D0); //"pl"
	}
	QAngle GetEyeAngles(){
		return *reinterpret_cast<QAngle*>((DWORD)this + (DWORD)0x16F4); //"m_vecViewOffset"
	}
	Vector GetViewOffset(){
		return *reinterpret_cast<Vector*>((DWORD)this + (DWORD)0x0CC); //"m_vecViewOffset [0CC] [0D0] [0D4]"
	}
	Vector GetBaseVelocity(){
		return *reinterpret_cast<Vector*>((DWORD)this + (DWORD)0xE4); //"m_vecBaseVelocity"
	}
	Vector GetEntityPos() const {
		return *reinterpret_cast<Vector*>((DWORD)this + (DWORD)0xFC);
	}
	Vector GetConstraintCenter(){
		return *reinterpret_cast<Vector*>((DWORD)this + (DWORD)0x1180); //"m_vecConstraintCenter"
	}
	long GetActiveWeapon() const {
		return *reinterpret_cast<long*>((DWORD)this + (DWORD)0xE1C); //"m_hActiveWeapon" 
	}
	long GetMyWeapons() const {
		return *reinterpret_cast<long*>((DWORD)this + (DWORD)0xD5C); //"m_hMyWeapons" 
	}
	long GetViewModel() {
		return *reinterpret_cast<long*>((DWORD)this + (DWORD)0x1160); //"m_hViewModel" 
	}
	const int isGhost() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x196F); //"m_isGhost" 
	}
	const int GetAmmo() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0xCDC); //"m_iAmmo" 
	}
	const int GetFOV() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x010E4); //"m_iFOV" 
	}
	const int GetFOVStart() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x010E8); //"m_iFOVStart"
	}	
	const float GetFOVTime() const {
		return *reinterpret_cast<float*>((DWORD)this + (DWORD)0x01108); //"m_flFOVTime"
	}
	const float GetVelocityModifier() const {
		return *reinterpret_cast<float*>((DWORD)this + (DWORD)0x163C); //"m_flVelocityModifier"
	}
	const int GetDefaultFOV() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x0116C); //"m_iDefaultFOV"
	}
	long GetZoomOwner() const {
		return *reinterpret_cast<long*>((DWORD)this + (DWORD)0x01154); //"m_hZoomOwner"
	}		
	long GetVehicle() const {
		return *reinterpret_cast<long*>((DWORD)this + (DWORD)0x01158); //"m_hVehicle"
	}
	long GetUseEntity() const {
		return *reinterpret_cast<long*>((DWORD)this + (DWORD)0x011FC); //"m_hUseEntity"
	}
	long GetViewEntityy()   {
		return *reinterpret_cast<long*>((DWORD)this + (DWORD)0x1178); //"m_hViewEntity" 
	}
	long GetGroundEntity() const {
		return *reinterpret_cast<long*>((DWORD)this + (DWORD)0x0114); //"m_hGroundEntity"
	}		
	long GetElevator() const {
		return *reinterpret_cast<long*>((DWORD)this + (DWORD)0x011C4); //"m_hElevator"
	}
	const int GetHeightAboveElevator() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x011F4); //"m_flHeightAboveElevator"
	}
	const int GetBuffer() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x1C84); //"m_healthBuffer"
	}
	const int GetHealth() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0xc4); //"m_iHealth"
	}
	const int GetTeam() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x0bc); //"m_iTeam"
	}
	int GetCollisionGroup() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x324); //"m_CollisionGroup"
	}
	const int GetZobbieClass() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x1964); //"m_iZobbieClass"
	}
	const int GetSurvivorCharacter() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x1960); //"m_survivorCharacter"
	}
	const int GetAddonBits() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x15F4); //"m_iAddonBits"
	}
	const int GetPrimaryAddon() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x15F8); //"m_iPrimaryAddon"
	}
	const int GetSecondaryAddon() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x15FC); //"m_iSecondaryAddon"
	}
	const int GetPlayerState() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x15E8); //"m_iPlayerState"
	}
	const int GetClass() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x16EC); //"m_iClass"
	}
	const int GetZobbieState() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x1968); //"m_zombieState"
	}
	const int GetGhostSpawnState() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x1970); //"m_ghostSpawnState"
	}
	const BYTE GetLifeState() const {
		return *reinterpret_cast<BYTE*>((DWORD)this + (DWORD)0x011F); //"m_lifeState"
	}
	bool IsAlive() const {
		return GetLifeState() == LIFE_ALIVE; ; //"IsAlive"
	}
	const int GetBonusProgress() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x01148); //"m_iBonusProgress"
	}
	const int GetBonusChallenge() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x0114C); //"m_iBonusChallenge"
	}
	const float GetMaxspeed() const {
		return *reinterpret_cast<float*>((DWORD)this + (DWORD)0x01150); //"m_flMaxspeed"
	}
	const int GetFlag() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x0C8); //"m_fFlags"
	}
	const int GetObserverMode() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x0119C); //"m_iObserverMode"
	}		
	const int GetObserverTarget() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x011A0); //"m_hObserverTarget"
	}	
	const char* GetCustomName() {
		return (char*)((DWORD)this + (DWORD)0x013C0); //"m_szLastPlaceName" 12 byte
	}		
	const Vector GetLadderNormal() const {
		return *reinterpret_cast<Vector*>((DWORD)this + (DWORD)0x0111C); //"m_vecLadderNormal"
	}		
	const int GetladderSurfaceProps() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x01100); //"m_ladderSurfaceProps"
	}	
	const BYTE Get_ubEFNoInterpParity() const {
		return *reinterpret_cast<BYTE*>((DWORD)this + (DWORD)0x013E4); //"m_ubEFNoInterpParity"
	}
	const int GetPostProcessCtrl() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x0157C); //"m_hPostProcessCtrl"
	}
	const int GetColorCorrectionCtrl() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x01580); //"m_hColorCorrectionCtrl"
	}
	const int GetFogCtrl() const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)0x01588); //"m_hFogCtrl"
	}
	const int GetInt(int i) const {
		return *reinterpret_cast<int*>((DWORD)this + (DWORD)i);
	}
	const char* GetCustomName(int i) {
		return (char*)((DWORD)this + i);
	}
	const BYTE GetByte(int i) const {
		return *reinterpret_cast<BYTE*>((DWORD)this + i);
	}
	const float Getfloat(int i) const {
		return *reinterpret_cast<float*>((DWORD)this + i);
	}
	void GetPos(int i, float * myCoords) const {
		myCoords[0] = Getfloat(i),myCoords[1] = Getfloat(i+4),myCoords[2] = Getfloat(i+8);
	}
	Vector GetOrign() {
		//return *(Vector*) ((DWORD)this + (DWORD)0xFC);
		return *reinterpret_cast<Vector*>((DWORD)this + 0xFC);
	}
	void SetFov( const int& newFov ) {
		*reinterpret_cast<int*>((DWORD)this + (DWORD)0x010E4) = newFov;
	}
};
