//=============================================================================//
//
// Purpose: Interface server dll virtual functions to the SDK.
//
// $NoKeywords: $
//=============================================================================//
#ifndef GAMEINTERFACE_H
#define GAMEINTERFACE_H
#include "public/eiface.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class ServerClass;

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
class CServerGameDLL
{
public:
	void GameInit(void);
	void PrecompileScriptsJob(void);
	void LevelShutdown(void);
	void GameShutdown(void);
	float GetTickInterval(void);
	ServerClass* GetAllServerClasses(void);

	static void __fastcall OnReceivedSayTextMessage(void* thisptr, int senderId, const char* text, bool isTeamChat);
};

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
class CServerGameClients : public IServerGameClients
{
};

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
class CServerGameEnts : public IServerGameEnts
{
};

inline CMemory p_CServerGameDLL__OnReceivedSayTextMessage;
inline auto CServerGameDLL__OnReceivedSayTextMessage = p_CServerGameDLL__OnReceivedSayTextMessage.RCast<void(__fastcall*)(void* thisptr, int senderId, const char* text, bool isTeamChat)>();

inline CMemory p_RunFrameServer;
inline auto v_RunFrameServer = p_RunFrameServer.RCast<void(*)(double flFrameTime, bool bRunOverlays, bool bUniformUpdate)>();

extern CServerGameDLL* g_pServerGameDLL;
extern CServerGameClients* g_pServerGameClients;
extern CServerGameEnts* g_pServerGameEntities;

extern CGlobalVars** g_pGlobals;

void CServerGameDLL_Attach();
void CServerGameDLL_Detach();

///////////////////////////////////////////////////////////////////////////////
class VServerGameDLL : public IDetour
{
	virtual void GetAdr(void) const
	{
		spdlog::debug("| FUN: OnReceivedSayTextMessage             : {:#18x} |\n", p_CServerGameDLL__OnReceivedSayTextMessage.GetPtr());
		spdlog::debug("| FUN: RunFrameServer                       : {:#18x} |\n", p_RunFrameServer.GetPtr());
		spdlog::debug("| VAR: g_pServerGameDLL                     : {:#18x} |\n", reinterpret_cast<uintptr_t>(g_pServerGameDLL));
		spdlog::debug("| VAR: g_pServerGameClients                 : {:#18x} |\n", reinterpret_cast<uintptr_t>(g_pServerGameClients));
		spdlog::debug("| VAR: g_pServerGameEntities                : {:#18x} |\n", reinterpret_cast<uintptr_t>(g_pServerGameEntities));
		spdlog::debug("| VAR: g_pGlobals                           : {:#18x} |\n", reinterpret_cast<uintptr_t>(g_pGlobals));
		spdlog::debug("+----------------------------------------------------------------+\n");
	}
	virtual void GetFun(void) const
	{
#if defined(GAMEDLL_S3)
		p_CServerGameDLL__OnReceivedSayTextMessage = g_GameDll.FindPatternSIMD("85 D2 0F 8E ?? ?? ?? ?? 4C 8B DC");
		CServerGameDLL__OnReceivedSayTextMessage = p_CServerGameDLL__OnReceivedSayTextMessage.RCast<void(__fastcall*)(void* thisptr, int senderId, const char* text, bool isTeamChat)>();

		p_RunFrameServer = g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 57 48 83 EC 30 0F 29 74 24 ?? 48 8D 0D ?? ?? ?? ??");
		v_RunFrameServer = p_RunFrameServer.RCast<void(*)(double, bool, bool)>();
#endif
	}
	virtual void GetVar(void) const
	{
		g_pGlobals = g_GameDll.FindPatternSIMD("4C 8B 0D ?? ?? ?? ?? 48 8B D1").ResolveRelativeAddressSelf(0x3, 0x7).RCast<CGlobalVars**>();
	}
	virtual void GetCon(void) const { }
	virtual void Attach(void) const { }
	virtual void Detach(void) const { }
};
///////////////////////////////////////////////////////////////////////////////

REGISTER(VServerGameDLL);

#endif // GAMEINTERFACE_H