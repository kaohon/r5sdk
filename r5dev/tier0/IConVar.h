#pragma once
#include "tier0/cmd.h"
#include "mathlib/color.h"

//-----------------------------------------------------------------------------
// Command to ConVars and ConCommands
//-----------------------------------------------------------------------------
// ConVar systems
#define FCVAR_NONE                   0	// The default, no flags at all
#define FCVAR_UNREGISTERED      (1<<0)	// If this is set, don't add to linked list, etc.
#define FCVAR_DEVELOPMENTONLY   (1<<1)	// Hidden in released products. Flag is removed automatically if ALLOW_DEVELOPMENT_CVARS is defined.
#define FCVAR_GAMEDLL           (1<<2)	// defined by the game DLL
#define FCVAR_CLIENTDLL         (1<<3)	// defined by the client DLL
#define FCVAR_HIDDEN            (1<<4)	// Hidden. Doesn't appear in find or auto complete. Like DEVELOPMENTONLY, but can't be compiled out.

// ConVar only
#define FCVAR_PROTECTED         (1<<5)	// It's a server cvar, but we don't send the data since it's a password, etc.  Sends 1 if it's not bland/zero, 0 otherwise as value
#define FCVAR_SPONLY            (1<<6)	// This cvar cannot be changed by clients connected to a multiplayer server.
#define	FCVAR_ARCHIVE           (1<<7)	// set to cause it to be saved to vars.rc
#define	FCVAR_NOTIFY            (1<<8)	// notifies players when changed
#define	FCVAR_USERINFO          (1<<9)	// changes the client's info string

#define FCVAR_PRINTABLEONLY     (1<<10)	// This cvar's string cannot contain unprintable characters ( e.g., used for player name etc ).

#define FCVAR_GAMEDLL_FOR_REMOTE_CLIENTS        (1<<10)  // When on concommands this allows remote clients to execute this cmd on the server. 
														 // We are changing the default behavior of concommands to disallow execution by remote clients without
														 // this flag due to the number existing concommands that can lag or crash the server when clients abuse them.

#define FCVAR_UNLOGGED          (1<<11)  // If this is a FCVAR_SERVER, don't log changes to the log file / console if we are creating a log
#define FCVAR_NEVER_AS_STRING   (1<<12)  // never try to print that cvar

// It's a ConVar that's shared between the client and the server.
// At signon, the values of all such ConVars are sent from the server to the client (skipped for local client, of course )
// If a change is requested it must come from the console (i.e., no remote client changes)
// If a value is changed while a server is active, it's replicated to all connected clients
#define FCVAR_REPLICATED        (1<<13)	// server setting enforced on clients, TODO rename to FCAR_SERVER at some time
#define FCVAR_CHEAT             (1<<14)	// Only useable in singleplayer / debug / multiplayer & sv_cheats
#define FCVAR_SS                (1<<15)	// causes varnameN where N == 2 through max splitscreen slots for mod to be autogenerated
#define FCVAR_DEMO              (1<<16)	// record this cvar when starting a demo file
#define FCVAR_DONTRECORD        (1<<17)	// don't record these command in demofiles
#define FCVAR_SS_ADDED          (1<<18)	// This is one of the "added" FCVAR_SS variables for the splitscreen players
#define FCVAR_RELEASE           (1<<19)	// Cvars tagged with this are the only cvars avaliable to customers
#define FCVAR_RELOAD_MATERIALS  (1<<20)	// If this cvar changes, it forces a material reload
#define FCVAR_RELOAD_TEXTURES   (1<<21)	// If this cvar changes, if forces a texture reload

#define FCVAR_NOT_CONNECTED          (1<<22)	// cvar cannot be changed by a client that is connected to a server
#define FCVAR_MATERIAL_SYSTEM_THREAD (1<<23)	// Indicates this cvar is read from the material system thread
#define FCVAR_ARCHIVE_GAMECONSOLE    (1<<24)	// cvar written to config.cfg on the Xbox

#define FCVAR_SERVER_CAN_EXECUTE    (1<<28)	// the server is allowed to execute this command on clients via ClientCommand/NET_StringCmd/CBaseClientState::ProcessStringCmd.
#define FCVAR_SERVER_CANNOT_QUERY   (1<<29)	// If this is set, then the server is not allowed to query this cvar's value (via IServerPluginHelpers::StartQueryCvarValue).
#define FCVAR_CLIENTCMD_CAN_EXECUTE (1<<30)	// IVEngineClient::ClientCmd is allowed to execute this command.

/*
class ConVar : ConCommandBase, IConVar; [MI] (#classinformer)
dq offset ? ? _R4ConVar@@6B@; const ConVar::`RTTI Complete Object Locator'

dq offset ??_G__ExceptionPtr@@QEAAPEAXI@Z_0; 0 Index
dq offset sub_1401F9930
dq offset loc_14046FE90
dq offset ConVar__AddFlags
dq offset ConVar__RemoveFlags
dq offset sub_14046FEA0
dq offset loc_14046FF70
dq offset ConVar__GetHelpString
dq offset sub_14046FEC0
dq offset sub_14046FEE0
dq offset ConVar__IsRegistered
dq offset ConVar__GetDllIdentifier
dq offset sub_14046F3F0
dq offset sub_14046F470
dq offset ConVar__InternalSetFloatValue; The one below also does something similar
dq offset sub_140470340
dq offset sub_140470420; Seems to be InternalSetInt below maybe ?
dq offset sub_140470510
dq offset nullsub
dq offset sub_140470300
dq offset sub_1404701A0
dq offset RegisterConVar; #STR: "Convar '%s' is flagged as both FCVAR_ARCHIVE and FCVAR_ARC
*/

//-----------------------------------------------------------------------------
// Purpose: A console variable
//-----------------------------------------------------------------------------
class ConVar
{
public:
	ConVar(void){};
	ConVar(const char* pszName, const char* pszDefaultValue, int nFlags, const char*pszHelpString,
		bool bMin, float fMin, bool bMax, float fMax, void* pCallback, const char* pszUsageString);
	~ConVar(void);

	void Init(void) const;

	void AddFlags(int nFlags);
	void RemoveFlags(int nFlags);

	const char* GetBaseName(void) const;
	const char* GetHelpText(void) const;
	const char* GetUsageText(void) const;

	bool GetBool(void) const;
	float GetFloat(void) const;
	int GetInt(void) const;
	Color GetColor(void) const;
	const char* GetString(void) const;

	bool GetMin(float& flMinValue) const;
	bool GetMax(float& flMaxValue) const;
	float GetMinValue(void) const;
	float GetMaxValue(void) const;
	bool HasMin(void) const;
	bool HasMax(void) const;

	void SetValue(int nValue);
	void SetValue(float flValue);
	void SetValue(const char* pszValue);
	void SetValue(Color clValue);

	void Revert(void);

	const char* GetDefault(void) const;
	void SetDefault(const char* pszDefault);

	void ChangeStringValue(const char* pszTempValue, float flOldValue);
	bool SetColorFromString(const char* pszValue);
	bool ClampValue(float& value);

	bool IsRegistered(void) const;
	bool IsCommand(void) const;
	static bool IsFlagSet(ConVar* pConVar, int nFlags);

	void ClearHostNames(void);

	struct CVValue_t
	{
		const char* m_pszString;
		int64_t     m_iStringLength;
		float       m_fValue;
		int         m_nValue;
	};

	ConCommandBase m_ConCommandBase {}; //0x0000
	void*          m_pIConVarVTable {}; //0x0040
	ConVar*        m_pParent        {}; //0x0048
	const char*    m_pszDefaultValue{}; //0x0050
	CVValue_t      m_Value          {}; //0c0058
	bool           m_bHasMin        {}; //0x0070
	float          m_fMinVal        {}; //0x0074
	bool           m_bHasMax        {}; //0x0078
	float          m_fMaxVal        {}; //0x007C
	char           pad_0080[32]     {}; //0x0080
}; //Size: 0x00A0

namespace
{
	/* ==== ICONVAR ========================================================================================================================================================= */
	ADDRESS p_IConVar_IsFlagSet = g_mGameDll.FindPatternSIMD((std::uint8_t*)"\x48\x8B\x41\x48\x85\x50\x38", "xxxxxxx");
	bool (*IConVar_IsFlagSet)(ConVar* pConVar, int nFlag) = (bool (*)(ConVar*, int))p_IConVar_IsFlagSet.GetPtr(); /*48 8B 41 48 85 50 38*/

	ADDRESS p_ConVar_SetInfo = g_mGameDll.FindPatternSIMD((std::uint8_t*)"\x40\x53\x48\x83\xEC\x60\x48\x8B\xD9\xC6\x41\x10\x00\x33\xC9\x48\x8D\x05\x00\x00\x00\x00\x48\x89\x4C\x24\x00\x0F\x57\xC0\x48\x89\x4C\x24\x00\x48\x89\x03\x48\x8D\x05\x00\x00\x00\x00\x48\x89\x43\x40", "xxxxxxxxxxxxxxxxxx????xxxx?xxxxxxx?xxxxxx????xxxx");
	void* (*ConVar_SetInfo)(void* a1, int a2, int a3, int a4, void* a5) = (void* (*)(void* a1, int a2, int a3, int a4, void* a5))p_ConVar_SetInfo.GetPtr(); /*40 53 48 83 EC 60 48 8B D9 C6 41 10 00 33 C9 48 8D 05 ? ? ? ? 48 89 4C 24 ? 0F 57 C0 48 89 4C 24 ? 48 89 03 48 8D 05 ? ? ? ? 48 89 43 40*/
#if defined (GAMEDLL_S0) || defined (GAMEDLL_S1)
	ADDRESS p_ConVar_Register = g_mGameDll.FindPatternSIMD((std::uint8_t*)"\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x48\x89\x7C\x24\x00\x41\x56\x48\x83\xEC\x30\xF3\x0F\x10\x44\x24\x00", "xxxx?xxxx?xxxx?xxxx?xxxxxxxxxxx?"); /*48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC 30 F3 0F 10 44 24 ?*/
	void* (*ConVar_Register)(ConVar* allocatedConvar, const char* szName, const char* szDefaultValue, int nFlags, const char* szHelpString, bool bMin, float fMin, bool bMax, float fMax, void* pCallback, void* unk) = (void* (*)(ConVar*, const char*, const char*, int, const char*, bool, float, bool, float, void*, void*))p_ConVar_Register.GetPtr();
#elif defined (GAMEDLL_S2) || defined (GAMEDLL_S3)
	ADDRESS p_ConVar_Register = g_mGameDll.FindPatternSIMD((std::uint8_t*)"\x48\x89\x5C\x24\x00\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x57\x48\x83\xEC\x40\xF3\x0F\x10\x84\x24\x00\x00\x00\x00", "xxxx?xxxx?xxxx?xxxxxxxxxx????"); /*48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 40 F3 0F 10 84 24 ? ? ? ?*/
	void* (*ConVar_Register)(ConVar* allocatedConvar, const char* szName, const char* szDefaultValue, int nFlags, const char* szHelpString, bool bMin, float fMin, bool bMax, float fMax, void* pCallback, const char* pszUsageString) = (void* (*)(ConVar*, const char*, const char*, int, const char*, bool, float, bool, float, void*, const char*))p_ConVar_Register.GetPtr();
#endif
}

namespace
{
	ADDRESS g_pConVarVtable  = p_ConVar_SetInfo.Offset(0x00).FindPatternSelf("48 8D 05", ADDRESS::Direction::DOWN, 100).ResolveRelativeAddressSelf(0x3, 0x7).GetPtr(); // Get vtable ptr for ConVar table.
	ADDRESS g_pIConVarVtable = p_ConVar_SetInfo.Offset(0x16).FindPatternSelf("48 8D 05", ADDRESS::Direction::DOWN, 100).ResolveRelativeAddressSelf(0x3, 0x7).GetPtr(); // Get vtable ptr for ICvar table.
}

///////////////////////////////////////////////////////////////////////////////
void IConVar_Attach();
void IConVar_Detach();

extern ConVar* g_pConVar;

///////////////////////////////////////////////////////////////////////////////
class HConVar : public IDetour
{
	virtual void debugp()
	{
		std::cout << "| FUN: IConVar::IsFlagSet                   : 0x" << std::hex << std::uppercase << p_IConVar_IsFlagSet.GetPtr() << std::setw(npad) << " |" << std::endl;
		std::cout << "| FUN: IConVar::SetInfo                     : 0x" << std::hex << std::uppercase << p_ConVar_SetInfo.GetPtr()    << std::setw(npad) << " |" << std::endl;
		std::cout << "| FUN: IConVar::Register                    : 0x" << std::hex << std::uppercase << p_ConVar_Register.GetPtr()   << std::setw(npad) << " |" << std::endl;
		std::cout << "| VAR: g_pConVarVtable                      : 0x" << std::hex << std::uppercase << g_pConVarVtable.GetPtr()     << std::setw(npad) << " |" << std::endl;
		std::cout << "| VAR: g_pIConVarVtable                     : 0x" << std::hex << std::uppercase << g_pIConVarVtable.GetPtr()    << std::setw(npad) << " |" << std::endl;
		std::cout << "+----------------------------------------------------------------+" << std::endl;
	}
};
///////////////////////////////////////////////////////////////////////////////

REGISTER(HConVar);
