#pragma once

//-----------------------------------------------------------------------------
// Purpose: Command tokenizer
//-----------------------------------------------------------------------------
class CCommand
{
private:
	enum
	{
		COMMAND_MAX_ARGC   = 64,
		COMMAND_MAX_LENGTH = 512,
	};

public:
	CCommand() = delete;

	int MaxCommandLength();
	std::int64_t ArgC(void) const;
	const char** ArgV(void) const;
	const char* ArgS(void) const;
	const char* GetCommandString(void) const;
	const char* Arg(int nIndex) const;
	const char* operator[](int nIndex) const;

private:
	std::int64_t m_nArgc;
	std::int64_t m_nArgv0Size;
	char         m_pArgSBuffer[COMMAND_MAX_LENGTH];
	char         m_pArgvBuffer[COMMAND_MAX_LENGTH];
	const char*  m_ppArgv[COMMAND_MAX_ARGC];
};

//-----------------------------------------------------------------------------
// Purpose: The console invoked command
//-----------------------------------------------------------------------------
class ConCommand
{
	friend class CCVar;
public:
	ConCommand(void) {};
	ConCommand(const char* szName, const char* szHelpString, int nFlags, void* pCallback, void* pCommandCompletionCallback);
	void Init(void);
	// TODO
};

//-----------------------------------------------------------------------------
// Purpose: The base console invoked command/cvar interface
//-----------------------------------------------------------------------------
class ConCommandBase
{
public:
	void AddFlags(int nFlags);
	void RemoveFlags(int nFlags);
	bool HasFlags(int nFlags);
	static bool IsFlagSet(ConCommandBase* pCommandBase, int nFlags);

	void* m_pConCommandBaseVTable; //0x0000
	ConCommandBase* m_pNext;       //0x0008
	bool        m_bRegistered;     //0x0010
private:
	char        pad_0011[7];       //0x0011
public:
	const char* m_pszName;         //0x0018
	const char* m_pszHelpString;   //0x0020
private:
	char        pad_0028[16];      //0x0028
public:
	int         m_nFlags;          //0x0038
private:
	char       pad_003C[4];        //0x003C
}; //Size: 0x0038

namespace
{
	/* ==== CONCOMMAND ====================================================================================================================================================== */
	ADDRESS p_ConCommandBase_IsFlagSet = g_mGameDll.FindPatternSIMD((std::uint8_t*)"\x85\x51\x38\x0F\x95\xC0\xC3", "xxxxxxx");
	bool (*ConCommandBase_IsFlagSet)(ConCommandBase* cmd, int flag) = (bool (*)(ConCommandBase*, int))p_ConCommandBase_IsFlagSet.GetPtr(); /*85 51 38 0F 95 C0 C3*/

	ADDRESS p_ConCommand_CMaterialSystemCmdInit = g_mGameDll.FindPatternSIMD((std::uint8_t*)"\x48\x89\x5C\x24\x00\x48\x89\x74\x24\x00\x48\x89\x7C\x24\x00\x55\x41\x54\x41\x55\x41\x56\x41\x57\x48\x8B\xEC\x48\x83\xEC\x50\x48\x8B\x15\x00\x00\x00\x00", "xxxx?xxxx?xxxx?xxxxxxxxxxxxxxxxxxx????");
	ConCommand*(*ConCommand_CMaterialSystemCmdInit)() = (ConCommand* (*)())p_ConCommand_CMaterialSystemCmdInit.GetPtr();

	ADDRESS p_ConCommand_NullSub = g_mGameDll.FindPatternSIMD((std::uint8_t*)"\xC2\x00\x00\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x40\x53\x48\x83\xEC\x20\x48\x8D\x05\x00\x00\x00\x00", "xxxxxxxxxxxxxxxxxxxxxxxxx????");
	void (*ConCommand_NullSub)() = (void (*)())p_ConCommand_NullSub.GetPtr(); /*C2 00 00 CC CC CC CC CC CC CC CC CC CC CC CC CC 40 53 48 83 EC 20 48 8D 05 ?? ?? ?? ??*/

	ADDRESS p_ConCommand_CallbackCompletion = g_mGameDll.FindPatternSIMD((std::uint8_t*)"\x33\xC0\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x80\x49\x68\x08", "xxxxxxxxxxxxxxxxxxxx");
	void* (*ConCommand_CallbackCompletion)(struct _exception* _exc) = (void* (*)(struct _exception*))p_ConCommand_CallbackCompletion.GetPtr(); /*33 C0 C3 CC CC CC CC CC CC CC CC CC CC CC CC CC 80 49 68 08*/ /*UserMathErrorFunction*/

	ADDRESS p_ConCommand_RegisterConCommand = g_mGameDll.FindPatternSIMD((std::uint8_t*)"\x48\x8B\xD1\x48\x8B\x0D\x00\x00\x00\x00\x48\x85\xC9\x74\x06", "xxxxxx????xxxxx");
	void* (*ConCommand_RegisterConCommand)(ConCommandBase* pCommandBase) = (void* (*)(ConCommandBase*))p_ConCommand_RegisterConCommand.GetPtr(); /*48 8B D1 48 8B 0D ?? ?? ?? ?? 48 85 C9 74 06 */

	static ADDRESS g_pConCommandVtable = p_ConCommand_CMaterialSystemCmdInit.FindPatternSelf("4C 8D 25", ADDRESS::Direction::DOWN, 150).ResolveRelativeAddressSelf(0x3, 0x7);
}

///////////////////////////////////////////////////////////////////////////////
void ConCommand_Attach();
void ConCommand_Detach();

extern ConCommand* g_pConCommand;

///////////////////////////////////////////////////////////////////////////////
class HConCommand : public IDetour
{
	virtual void debugp()
	{
		std::cout << "| FUN: ConCommandBase::IsFlagSet            : 0x" << std::hex << std::uppercase << p_ConCommandBase_IsFlagSet.GetPtr()              << std::setw(npad) << " |" << std::endl;
		std::cout << "| FUN: ConCommand::CMaterialSystemCmdInit   : 0x" << std::hex << std::uppercase << p_ConCommand_CMaterialSystemCmdInit.GetPtr() << std::setw(npad) << " |" << std::endl;
		std::cout << "| FUN: ConCommand::NullSub                  : 0x" << std::hex << std::uppercase << p_ConCommand_NullSub.GetPtr()                << std::setw(npad) << " |" << std::endl;
		std::cout << "| FUN: ConCommand::CallbackCompletion       : 0x" << std::hex << std::uppercase << p_ConCommand_CallbackCompletion.GetPtr()     << std::setw(npad) << " |" << std::endl;
		std::cout << "| FUN: ConCommand::RegisterConCommand       : 0x" << std::hex << std::uppercase << p_ConCommand_RegisterConCommand.GetPtr()     << std::setw(npad) << " |" << std::endl;
		std::cout << "| VAR: g_pConCommandVtable                  : 0x" << std::hex << std::uppercase << g_pConCommandVtable.GetPtr()                 << std::setw(npad) << " |" << std::endl;
		std::cout << "+----------------------------------------------------------------+" << std::endl;
	}
};
///////////////////////////////////////////////////////////////////////////////

REGISTER(HConCommand);
