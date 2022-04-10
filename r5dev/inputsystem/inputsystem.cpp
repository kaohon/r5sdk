//===========================================================================//
//
// Purpose: 
//
//===========================================================================//

#include "core/stdafx.h"
#include "vpc/IAppSystem.h"
#include "inputsystem/inputsystem.h"

//-----------------------------------------------------------------------------
// Enables/disables input
//-----------------------------------------------------------------------------
void CInputSystem::EnableInput(bool bEnabled)
{
	static int index = 10;
	CallVFunc<void>(index, this, bEnabled);
}

//-----------------------------------------------------------------------------
// Enables/disables the inputsystem windows message pump
//-----------------------------------------------------------------------------
void CInputSystem::EnableMessagePump(bool bEnabled)
{
	static int index = 11;
	CallVFunc<void>(index, this, bEnabled);
}

//-----------------------------------------------------------------------------
// Poll current state
//-----------------------------------------------------------------------------
bool CInputSystem::IsButtonDown(ButtonCode_t Button)
{
	static int index = 13;
	return CallVFunc<bool>(index, this, Button);
}

///////////////////////////////////////////////////////////////////////////////
CInputSystem* g_pInputSystem = reinterpret_cast<CInputSystem*>(p_IAppSystem_LoadLibrary.FindPatternSelf("48 89 05", CMemory::Direction::DOWN, 40).ResolveRelativeAddressSelf(0x3, 0x7).GetPtr());