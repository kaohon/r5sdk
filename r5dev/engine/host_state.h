#pragma once
#include "mathlib/vector.h"

enum class HostStates_t : int
{
	HS_NEW_GAME        = 0x0,
	HS_LOAD_GAME       = 0x1,
	HS_CHANGE_LEVEL_SP = 0x2,
	HS_CHANGE_LEVEL_MP = 0x3,
	HS_RUN             = 0x4,
	HS_GAME_SHUTDOWN   = 0x5,
	HS_SHUTDOWN        = 0x6,
	HS_RESTART         = 0x7,
};

class CHostState
{
public:
	FORCEINLINE static void FrameUpdate(void* rcx, void* rdx, float time);
	FORCEINLINE void LoadConfig(void) const;

	FORCEINLINE void Setup(void) const;
	FORCEINLINE void Think(void) const;

	FORCEINLINE void GameShutDown(void);
	FORCEINLINE void UnloadPakFile(void);

	FORCEINLINE void State_NewGame(void);
	FORCEINLINE void State_ChangeLevelSP(void);
	FORCEINLINE void State_ChangeLevelMP(void);

public:
	HostStates_t m_iCurrentState;                    //0x0000
	HostStates_t m_iNextState;                       //0x0004
	Vector3      m_vecLocation;                      //0x0008
	QAngle       m_angLocation;                      //0x0014
	char         m_levelName[64];                    //0x0020
	char         m_mapGroupName[256];                //0x0060
	char         m_landMarkName[256];                //0x0160
	float        m_flShortFrameTime;                 //0x0260
	bool         m_bActiveGame;                      //0x0264
	bool         m_bRememberLocation;                //0x0265
	bool         m_bBackgroundLevel;                 //0x0266
	bool         m_bWaitingForConnection;            //0x0267
	bool         m_bSplitScreenConnect;              //0x0268
	bool         m_bGameHasShutDownAndFlushedMemory; //0x0269
	bool         m_bWorkshopMapDownloadPending;      //0x026A
};

/* ==== CHOSTSTATE ====================================================================================================================================================== */
inline CMemory p_CHostState_FrameUpdate = g_mGameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x48\x89\x5C\x24\x08\x48\x89\x6C\x24\x20\xF3\x0F\x11\x54\x24\x18"), "xxxxxxxxxxxxxxxx");
inline auto CHostState_FrameUpdate = p_CHostState_FrameUpdate.RCast<void(*)(void* rcx, void* rdx, float time)>(); /*48 89 5C 24 08 48 89 6C 24 20 F3 0F 11 54 24 18*/

namespace // !TEMP
{
	static auto setjmpFn            = CMemory(0x141205460).RCast<std::int64_t(*)(jmp_buf, void*)>();
	static auto host_abortserver    = CMemory(0x14B37C700).RCast<jmp_buf*>();
	static auto CHostState_InitFn   = CMemory(0x14023E7D0).RCast<void(*)(CHostState*)>();
	static auto g_ServerAbortServer = CMemory(0x14B37CA22).RCast<char*>();
	static auto State_RunFn         = CMemory(0x14023E870).RCast<void(*)(HostStates_t*, void*, float)>();
	static auto g_ServerGameClients = CMemory(0x14B383428).RCast<std::int64_t*>();
	static auto SV_InitGameDLLFn    = CMemory(0x140308B90).RCast<void(*)()>();
	static auto g_CModelLoader      = CMemory(0x14173B210).RCast<void*>();
	static auto CModelLoader_Map_IsValidFn = CMemory(0x1402562F0).RCast<bool(*)(void*, const char*)>();
	static auto Host_NewGameFn             = CMemory(0x140238DA0).RCast<bool(*)(char*, char*, bool, bool, void*)>();
	static auto Host_Game_ShutdownFn       = CMemory(0x14023EDA0).RCast<void(*)(CHostState*)>();
	static auto Host_ChangelevelFn             = CMemory(0x1402387B0).RCast<void(*)(bool, const char*, const char*)>();
	static auto CL_EndMovieFn                  = CMemory(0x1402C03D0).RCast<void(*)()>();
	static auto SendOfflineRequestToStryderFn  = CMemory(0x14033D380).RCast<void(*)()>();
}

extern bool g_bLevelResourceInitialized;
///////////////////////////////////////////////////////////////////////////////
void CHostState_Attach();
void CHostState_Detach();

///////////////////////////////////////////////////////////////////////////////
extern CHostState* g_pHostState;


///////////////////////////////////////////////////////////////////////////////
class HHostState : public IDetour
{
	virtual void debugp()
	{
		std::cout << "| FUN: CHostState::FrameUpdate              : 0x" << std::hex << std::uppercase << p_CHostState_FrameUpdate.GetPtr() << std::setw(npad) << " |" << std::endl;
		std::cout << "| VAR: g_pHostState                         : 0x" << std::hex << std::uppercase << g_pHostState                      << std::setw(0)    << " |" << std::endl;
		std::cout << "+----------------------------------------------------------------+" << std::endl;
	}
};
///////////////////////////////////////////////////////////////////////////////

REGISTER(HHostState);
