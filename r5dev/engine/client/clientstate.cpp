//=============================================================================//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
// clientstate.cpp: implementation of the CClientState class.
//
/////////////////////////////////////////////////////////////////////////////////
#include "core/stdafx.h"
#include "vpc/keyvalues.h"
#include "tier0/frametask.h"
#include "engine/host.h"
#include "clientstate.h"
#include "common/callback.h"
#include "cdll_engine_int.h"
#include "vgui/vgui_baseui_interface.h"
#include <ebisusdk/EbisuSDK.h>


//------------------------------------------------------------------------------
// Purpose: returns true if client simulation is paused
//------------------------------------------------------------------------------
bool CClientState::IsPaused() const
{
	return m_bPaused || !*host_initialized || g_pEngineVGui->ShouldPause();
}

//------------------------------------------------------------------------------
// Purpose: returns true if client is fully connected and active
//------------------------------------------------------------------------------
bool CClientState::IsActive(void) const
{
    return m_nSignonState == SIGNONSTATE::SIGNONSTATE_FULL;
};

//------------------------------------------------------------------------------
// Purpose: returns true if client connected but not active
//------------------------------------------------------------------------------
bool CClientState::IsConnected(void) const
{
    return m_nSignonState >= SIGNONSTATE::SIGNONSTATE_CONNECTED;
};

//------------------------------------------------------------------------------
// Purpose: returns true if client is still connecting
//------------------------------------------------------------------------------
bool CClientState::IsConnecting(void) const
{
    return m_nSignonState >= SIGNONSTATE::SIGNONSTATE_NONE;
}

//------------------------------------------------------------------------------
// Purpose: gets the client time
// Technically doesn't belong here
//------------------------------------------------------------------------------
float CClientState::GetClientTime() const
{
    if (m_bClockCorrectionEnabled)
    {
        return (float)m_ClockDriftMgr.m_nSimulationTick * g_pCommonHostState->interval_per_tick;
    }
    else
    {
        return m_flClockDriftFrameTime;
    }
}

//------------------------------------------------------------------------------
// Purpose: gets the simulation tick count
//------------------------------------------------------------------------------
int CClientState::GetTick() const
{
    return m_ClockDriftMgr.m_nSimulationTick;
}

//------------------------------------------------------------------------------
// Purpose: gets the last-received server tick count
//------------------------------------------------------------------------------
int CClientState::GetServerTickCount() const
{
    return m_ClockDriftMgr.m_nServerTick;
}

//------------------------------------------------------------------------------
// Purpose: sets the server tick count
//------------------------------------------------------------------------------
void CClientState::SetServerTickCount(int tick)
{
    m_ClockDriftMgr.m_nServerTick = tick;
}

//------------------------------------------------------------------------------
// Purpose: gets the client tick count
//------------------------------------------------------------------------------
int CClientState::GetClientTickCount() const
{
    return m_ClockDriftMgr.m_nClientTick;
}

//------------------------------------------------------------------------------
// Purpose: sets the client tick count
//------------------------------------------------------------------------------
void CClientState::SetClientTickCount(int tick)
{
    m_ClockDriftMgr.m_nClientTick = tick;
}

//------------------------------------------------------------------------------
// Purpose: gets the client frame time
//------------------------------------------------------------------------------
float CClientState::GetFrameTime() const
{
    if (IsPaused())
    {
        return 0.0f;
    }

    return m_flFrameTime;
}

//------------------------------------------------------------------------------
// Purpose: called when connection to the server has been closed
//------------------------------------------------------------------------------
void CClientState::VConnectionClosing(CClientState* thisptr, const char* szReason)
{
    CClientState__ConnectionClosing(thisptr, szReason);

    // Delay execution to the next frame; this is required to avoid a rare crash.
    // Cannot reload playlists while still disconnecting.
    g_TaskScheduler->Dispatch([]()
        {
            // Reload the local playlist to override the cached
            // one from the server we got disconnected from.
            _DownloadPlaylists_f();
            KeyValues::InitPlaylists();
        }, 0);
}

//------------------------------------------------------------------------------
// Purpose: called when a SVC_ServerTick messages comes in.
// This function has an additional check for the command tick against '-1',
// if it is '-1', we process statistics only. This is required as the game
// no longer can process server ticks every frame unlike previous games.
// Without this, the server CPU and frame time don't get updated to the client.
//------------------------------------------------------------------------------
bool CClientState::VProcessServerTick(CClientState* pClientState, SVC_ServerTick* pServerTick)
{
    if (pServerTick->m_NetTick.m_nCommandTick != -1)
    {
        return CClientState__ProcessServerTick(pClientState, pServerTick);
    }
    else // Statistics only.
    {
        char* pShifted = reinterpret_cast<char*>(pClientState) - 0x10; // Shifted due to compiler optimizations.
        CClientState* pClient_Adj = reinterpret_cast<CClientState*>(pShifted);

        CNetChan* pChan = pClient_Adj->m_NetChannel;
        pChan->SetRemoteFramerate(pServerTick->m_NetTick.m_flHostFrameTime, pServerTick->m_NetTick.m_flHostFrameTimeStdDeviation);
        pChan->SetRemoteCPUStatistics(pServerTick->m_NetTick.m_nServerCPU);

        return true;
    }
}

//------------------------------------------------------------------------------
// Purpose: get authentication token for current connection context
// Input  : *connectParams - 
// Output : true on success, false otherwise
//------------------------------------------------------------------------------
bool CClientState::Authenticate(connectparams_t* connectParams) const
{
    string msToken; // token returned by the masterserver authorising the client to play online
    string message; // message returned by the masterserver about the result of the auth

    // verify that the client is not lying about their account identity
    // code is immediately discarded upon verification

    const bool ret = g_pMasterServer->AuthForConnection(*g_NucleusID, connectParams->netAdr, g_OriginAuthCode, msToken, message);
    if (!ret)
    {
        Error(eDLL_T::ENGINE, NO_ERROR, "Failed to authenticate for online play: %s\n", message.c_str());
        return false;
    }

    // get full token
    const char* token = msToken.c_str();

    // get a pointer to the delimiter that begins the token's signature
    const char* tokenSignatureDelim = strrchr(token, '.');

    if (!tokenSignatureDelim)
    {
        Error(eDLL_T::ENGINE, NO_ERROR, "Failed to authenticate for online play: %s\n", "Invalid token returned by MS");
        return false;
    }

    // replace the delimiter with a null char so the first cvar only takes the header and payload data
    *(char*)tokenSignatureDelim = '\0';
    const size_t sigLength = strlen(tokenSignatureDelim) - 1;

    cl_onlineAuthToken->SetValue(token);

    if (sigLength > 0)
    {
        // get a pointer to the first part of the token signature to store in cl_onlineAuthTokenSignature1
        const char* tokenSignaturePart1 = tokenSignatureDelim + 1;

        cl_onlineAuthTokenSignature1->SetValue(tokenSignaturePart1);

        if (sigLength > 255)
        {
            // get a pointer to the rest of the token signature to store in cl_onlineAuthTokenSignature2
            const char* tokenSignaturePart2 = tokenSignaturePart1 + 255;

            cl_onlineAuthTokenSignature2->SetValue(tokenSignaturePart2);
        }
    }

    return true;
}

void CClientState::VConnect(CClientState* thisptr, connectparams_t* connectParams)
{
    if (strncmp(connectParams->netAdr, "localhost", 9) != NULL)
        thisptr->Authenticate(connectParams);

    CClientState__Connect(thisptr, connectParams);
}

void VClientState::Attach() const
{
    DetourAttach(&CClientState__ConnectionClosing, &CClientState::VConnectionClosing);
    DetourAttach(&CClientState__ProcessServerTick, &CClientState::VProcessServerTick);
    DetourAttach(&CClientState__Connect, &CClientState::VConnect);
}

void VClientState::Detach() const
{
    DetourDetach(&CClientState__ConnectionClosing, &CClientState::VConnectionClosing);
    DetourDetach(&CClientState__ProcessServerTick, &CClientState::VProcessServerTick);
    DetourDetach(&CClientState__Connect, &CClientState::VConnect);
}

/////////////////////////////////////////////////////////////////////////////////
CClientState* g_pClientState = nullptr;
CClientState** g_pClientState_Shifted = nullptr;
