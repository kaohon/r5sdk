#include "pch.h"
#include "hooks.h"

bool g_bBlockInput = false;

namespace Hooks
{
	bool bToggledDevFlags = true;
	bool bToggledNetTrace = false;
}

void Hooks::InstallHooks()
{
	///////////////////////////////////////////////////////////////////////////////
	// Initialize Minhook
	spdlog::debug("Hooking game functions now..\n");
	MH_Initialize();

	///////////////////////////////////////////////////////////////////////////////
	// Hook Squirrel functions
	MH_CreateHook(addr_SQVM_Print, &Hooks::SQVM_Print, NULL);
	MH_CreateHook(addr_SQVM_Warning, &Hooks::SQVM_Warning, reinterpret_cast<void**>(&originalSQVM_Warning));
	MH_CreateHook(addr_SQVM_LoadRson, &Hooks::SQVM_LoadRson, reinterpret_cast<void**>(&originalSQVM_LoadRson));
	MH_CreateHook(addr_SQVM_LoadScript, &Hooks::SQVM_LoadScript, reinterpret_cast<void**>(&originalSQVM_LoadScript));
	MH_CreateHook(addr_SQVM_RegisterOriginFuncs, &Hooks::SQVM_RegisterOriginFuncs, reinterpret_cast<void**>(&originalSQVM_RegisterOriginFuncs));
	MH_CreateHook(addr_SQVM_RegisterCreatePlayerTasklist, &Hooks::SQVM_RegisterCreatePlayerTasklist, reinterpret_cast<void**>(&originalSQVM_RegisterCreatePlayerTasklist));

	///////////////////////////////////////////////////////////////////////////////
	// Hook Game functions
	MH_CreateHook(addr_CHLClient_FrameStageNotify, &Hooks::FrameStageNotify, reinterpret_cast<void**>(&originalFrameStageNotify));
	MH_CreateHook(addr_CVEngineServer_IsPersistenceDataAvailable, &Hooks::IsPersistenceDataAvailable, reinterpret_cast<void**>(&originalIsPersistenceDataAvailable));
	MH_CreateHook(addr_CServer_ConnectClient, &Hooks::ConnectClient, reinterpret_cast<void**>(&originalConnectClient));

	///////////////////////////////////////////////////////////////////////////////
	// Hook Netchan functions
	MH_CreateHook(addr_NET_PrintFunc, &Hooks::NET_PrintFunc, reinterpret_cast<void**>(&originalNET_PrintFunc));
	MH_CreateHook(addr_NET_ReceiveDatagram, &Hooks::NET_ReceiveDatagram, reinterpret_cast<void**>(&originalNET_ReceiveDatagram));
	MH_CreateHook(addr_NET_SendDatagram, &Hooks::NET_SendDatagram, reinterpret_cast<void**>(&originalNET_SendDatagram));
	MH_CreateHook(addr_NetChan_Shutdown, &Hooks::NetChan_Shutdown, reinterpret_cast<void**>(&originalNetChan_ShutDown));

	///////////////////////////////////////////////////////////////////////////////
	// Hook ConVar | ConCommand functions
	MH_CreateHook(addr_ConVar_IsFlagSet, &Hooks::ConVar_IsFlagSet, reinterpret_cast<void**>(&originalConVar_IsFlagSet));
	MH_CreateHook(addr_ConCommand_IsFlagSet, &Hooks::ConCommand_IsFlagSet, reinterpret_cast<void**>(&originalConCommand_IsFlagSet));

	///////////////////////////////////////////////////////////////////////////////
	// Hook CMatSystemSurface functions
	MH_CreateHook(addr_CMatSystemSurface_LockCursor, &LockCursor, reinterpret_cast<void**>(&originalLockCursor));

	///////////////////////////////////////////////////////////////////////////////
	// Hook CBaseFileSystem functions
	//MH_CreateHook(addr_CBaseFileSystem_FileSystemWarning, &Hooks::FileSystemWarning, reinterpret_cast<void**>(&originalFileSystemWarning);

	///////////////////////////////////////////////////////////////////////////////
	// Hook HostState functions
    //MH_CreateHook(MemoryAddress(0x14023EF80).RCast<void*>(), &Hooks::FrameUpdate, reinterpret_cast<void**>(&originalFrameUpdate));

	///////////////////////////////////////////////////////////////////////////////
	// Hook Utility functions
	MH_CreateHook(addr_MSG_EngineError, &Hooks::MSG_EngineError, reinterpret_cast<void**>(&originalMSG_EngineError));
	MH_CreateHook(addr_LoadPlaylist, &Hooks::LoadPlaylist, reinterpret_cast<void**>(&originalLoadPlaylist));

	///////////////////////////////////////////////////////////////////////////////
	// Hook WinAPI
	if (Module user32dll = Module("user32.dll"); user32dll.GetModuleBase()) // Is user32.dll valid?
	{
		void* SetCursorPosPtr = user32dll.GetExportedFunction("SetCursorPos");
		void* ClipCursorPtr = user32dll.GetExportedFunction("ClipCursor");
		void* GetCursorPosPtr = user32dll.GetExportedFunction("GetCursorPos");
		void* ShowCursorPtr = user32dll.GetExportedFunction("ShowCursor");
		MH_CreateHook(SetCursorPosPtr, &Hooks::SetCursorPos, reinterpret_cast<void**>(&originalSetCursorPos));
		MH_CreateHook(ClipCursorPtr, &Hooks::ClipCursor, reinterpret_cast<void**>(&originalClipCursor));
		MH_CreateHook(GetCursorPosPtr, &Hooks::GetCursorPos, reinterpret_cast<void**>(&originalGetCursorPos));
		MH_CreateHook(ShowCursorPtr, &Hooks::ShowCursor, reinterpret_cast<void**>(&originalShowCursor));
		///////////////////////////////////////////////////////////////////////////
		// Enable WinAPI hooks
		MH_EnableHook(SetCursorPosPtr);
		MH_EnableHook(ClipCursorPtr);
		MH_EnableHook(GetCursorPosPtr);
		MH_EnableHook(ShowCursorPtr);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Enable Squirrel hooks
	MH_EnableHook(addr_SQVM_Print);
	MH_EnableHook(addr_SQVM_Warning);
	MH_EnableHook(addr_SQVM_LoadRson);
	MH_EnableHook(addr_SQVM_LoadScript);
	MH_EnableHook(addr_SQVM_RegisterOriginFuncs);
	MH_EnableHook(addr_SQVM_RegisterCreatePlayerTasklist);

	///////////////////////////////////////////////////////////////////////////////
	// Enable Game hooks
	MH_EnableHook(addr_CHLClient_FrameStageNotify);
	MH_EnableHook(addr_CVEngineServer_IsPersistenceDataAvailable);
	MH_EnableHook(addr_CServer_ConnectClient);

	///////////////////////////////////////////////////////////////////////////////
	// Enable Netchan hooks
	MH_EnableHook(addr_NET_PrintFunc);
	MH_EnableHook(addr_NetChan_Shutdown);

	///////////////////////////////////////////////////////////////////////////////
	// Enable ConVar | ConCommand hooks
	MH_EnableHook(addr_ConVar_IsFlagSet);
	MH_EnableHook(addr_ConCommand_IsFlagSet);

	///////////////////////////////////////////////////////////////////////////////
	// Enable CMatSystemSurface hooks
	MH_EnableHook(addr_CMatSystemSurface_LockCursor);

	///////////////////////////////////////////////////////////////////////////////
	// Enable CBaseFileSystem hooks
	//MH_EnableHook(addr_CBaseFileSystem_FileSystemWarning);

	///////////////////////////////////////////////////////////////////////////////
	// Enable HostState hooks
//	MH_EnableHook(MemoryAddress(0x14023EF80).RCast<void*>());

	///////////////////////////////////////////////////////////////////////////////
    // Enabled Utility hooks
	MH_EnableHook(addr_MSG_EngineError);
	MH_EnableHook(addr_LoadPlaylist);
}

void Hooks::RemoveHooks()
{
	spdlog::debug("Unhooking game functions..\n");
	///////////////////////////////////////////////////////////////////////////////
	// Unhook Squirrel functions
	MH_RemoveHook(addr_SQVM_Print);
	MH_RemoveHook(addr_SQVM_LoadRson);
	MH_RemoveHook(addr_SQVM_LoadScript);
	MH_RemoveHook(addr_SQVM_RegisterOriginFuncs);
	MH_RemoveHook(addr_SQVM_RegisterCreatePlayerTasklist);

	///////////////////////////////////////////////////////////////////////////////
	// Unhook Game Functions
	MH_RemoveHook(addr_CHLClient_FrameStageNotify);
	MH_RemoveHook(addr_CVEngineServer_IsPersistenceDataAvailable);
	MH_RemoveHook(addr_CServer_ConnectClient);

	///////////////////////////////////////////////////////////////////////////////
	// Unhook Netchan functions
	MH_RemoveHook(addr_NET_PrintFunc);
	MH_RemoveHook(addr_NET_ReceiveDatagram);
	MH_RemoveHook(addr_NET_SendDatagram);
	MH_RemoveHook(addr_NetChan_Shutdown);

	///////////////////////////////////////////////////////////////////////////////
	// Unhook ConVar | ConCommand functions
	MH_RemoveHook(addr_ConVar_IsFlagSet);
	MH_RemoveHook(addr_ConCommand_IsFlagSet);

	///////////////////////////////////////////////////////////////////////////////
	// Unhook CMatSystemSurface functions
	MH_RemoveHook(addr_CMatSystemSurface_LockCursor);

	///////////////////////////////////////////////////////////////////////////////
	// Unhook Utility functions
	MH_RemoveHook(addr_MSG_EngineError);
	MH_RemoveHook(addr_LoadPlaylist);

	///////////////////////////////////////////////////////////////////////////////
	// Unhook WinAPI
	if (Module user32dll = Module("user32.dll"); user32dll.GetModuleBase()) // Is user32.dll valid?
	{
		void* SetCursorPosPtr = user32dll.GetExportedFunction("SetCursorPos");
		void* ClipCursorPtr = user32dll.GetExportedFunction("ClipCursor");
		void* GetCursorPosPtr = user32dll.GetExportedFunction("GetCursorPos");
		void* ShowCursorPtr = user32dll.GetExportedFunction("ShowCursor");
		MH_RemoveHook(SetCursorPosPtr);
		MH_RemoveHook(ClipCursorPtr);
		MH_RemoveHook(GetCursorPosPtr);
		MH_RemoveHook(ShowCursorPtr);
	}

	///////////////////////////////////////////////////////////////////////////////
	// Unhook CBaseFileSystem functions.
	//MH_RemoveHook(addr_CBaseFileSystem_FileSystemWarning);

	///////////////////////////////////////////////////////////////////////////////
	// Unhook HostState hooks
    //MH_RemoveHook(MemoryAddress(0x14023EF80).RCast<void*>());

	///////////////////////////////////////////////////////////////////////////////
	// Reset Minhook
	MH_Uninitialize();
}

void Hooks::ToggleNetTrace()
{
	if (!bToggledNetTrace)
	{
		MH_EnableHook(addr_NET_ReceiveDatagram);
		MH_EnableHook(addr_NET_SendDatagram);
		spdlog::info("\n");
		spdlog::info("+--------------------------------------------------------+\n");
		spdlog::info("|>>>>>>>>>>>>>| NETCHANNEL TRACE ACTIVATED |<<<<<<<<<<<<<|\n");
		spdlog::info("+--------------------------------------------------------+\n");
		spdlog::info("\n");
	}
	else
	{
		MH_DisableHook(addr_NET_ReceiveDatagram);
		MH_DisableHook(addr_NET_SendDatagram);
		spdlog::info("\n");
		spdlog::info("+--------------------------------------------------------+\n");
		spdlog::info("|>>>>>>>>>>>>| NETCHANNEL TRACE DEACTIVATED |<<<<<<<<<<<<|\n");
		spdlog::info("+--------------------------------------------------------+\n");
		spdlog::info("\n");
	}
	bToggledNetTrace = !bToggledNetTrace;
}

void Hooks::ToggleDevCommands()
{
	if (!bToggledDevFlags)
	{
		MH_EnableHook(addr_ConVar_IsFlagSet);
		MH_EnableHook(addr_ConCommand_IsFlagSet);
		spdlog::info("\n");
		spdlog::info("+--------------------------------------------------------+\n");
		spdlog::info("|>>>>>>>>>>>>>| DEVONLY COMMANDS ACTIVATED |<<<<<<<<<<<<<|\n");
		spdlog::info("+--------------------------------------------------------+\n");
		spdlog::info("\n");
	}
	else
	{
		MH_DisableHook(addr_ConVar_IsFlagSet);
		MH_DisableHook(addr_ConCommand_IsFlagSet);
		spdlog::info("\n");
		spdlog::info("+--------------------------------------------------------+\n");
		spdlog::info("|>>>>>>>>>>>>| DEVONLY COMMANDS DEACTIVATED |<<<<<<<<<<<<|\n");
		spdlog::info("+--------------------------------------------------------+\n");
		spdlog::info("\n");
	}
	bToggledDevFlags = !bToggledDevFlags;
}
