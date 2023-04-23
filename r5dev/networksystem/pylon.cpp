//=====================================================================================//
//
// Purpose: Implementation of the pylon server backend.
//
// $NoKeywords: $
//=====================================================================================//

#include <core/stdafx.h>
#include <tier1/cvar.h>
#include <tier2/curlutils.h>
#include <networksystem/pylon.h>
#ifndef CLIENT_DLL
#include <engine/server/server.h>
#endif // !CLIENT_DLL

//-----------------------------------------------------------------------------
// Purpose: returns a vector of hosted servers.
// Input  : &svOutMessage - 
// Output : vector<NetGameServer_t>
//-----------------------------------------------------------------------------
vector<NetGameServer_t> CPylon::GetServerList(string& svOutMessage) const
{
    vector<NetGameServer_t> vslList;

    nlohmann::json jsRequestBody = nlohmann::json::object();
    jsRequestBody["version"] = SDK_VERSION;

    const string svRequestBody = jsRequestBody.dump(4);
    const bool bDebug = pylon_showdebuginfo->GetBool();

    if (bDebug)
    {
        DevMsg(eDLL_T::ENGINE, "%s - Sending server list request to comp-server:\n%s\n", __FUNCTION__, svRequestBody.c_str());
    }

    string svResponse;
    CURLINFO status;

    if (!QueryMasterServer(pylon_matchmaking_hostname->GetString(), "/servers", svRequestBody, svResponse, svOutMessage, status))
    {
        return vslList;
    }

    try
    {
        if (status == 200) // STATUS_OK
        {
            nlohmann::json jsResultBody = nlohmann::json::parse(svResponse);
            if (jsResultBody["success"].is_boolean() && jsResultBody["success"].get<bool>())
            {
                for (auto& obj : jsResultBody["servers"])
                {
                    vslList.push_back(
                        NetGameServer_t
                        {
                            obj.value("name",""),
                            obj.value("description",""),
                            obj.value("hidden","false") == "true",
                            obj.value("map",""),
                            obj.value("playlist",""),
                            obj.value("ip",""),
                            obj.value("port", ""),
                            obj.value("key",""),
                            obj.value("checksum",""),
                            obj.value("version", SDK_VERSION),
                            obj.value("playerCount", ""),
                            obj.value("maxPlayers", ""),
                            obj.value("timeStamp", 0),
                            obj.value("publicRef", ""),
                            obj.value("cachedId", ""),
                        }
                    );
                }
            }
            else
            {
                ExtractError(jsResultBody, svOutMessage, status);
            }
        }
        else
        {
            ExtractError(svResponse, svOutMessage, status, "Server list error");
            return vslList;
        }
    }
    catch (const std::exception& ex)
    {
        Warning(eDLL_T::ENGINE, "%s - Exception while parsing comp-server response:\n%s\n", __FUNCTION__, ex.what());
    }

    return vslList;
}

//-----------------------------------------------------------------------------
// Purpose: Gets the server by token string.
// Input  : &slOutServer - 
//			&svOutMessage - 
//			&svToken - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CPylon::GetServerByToken(NetGameServer_t& slOutServer, string& svOutMessage, const string& svToken) const
{
    nlohmann::json jsRequestBody = nlohmann::json::object();
    jsRequestBody["token"] = svToken;

    const string svRequestBody = jsRequestBody.dump(4);
    const bool bDebugLog = pylon_showdebuginfo->GetBool();

    if (bDebugLog)
    {
        DevMsg(eDLL_T::ENGINE, "%s - Sending token connect request to comp-server:\n%s\n", __FUNCTION__, svRequestBody.c_str());
    }

    string svResponseBuf;
    CURLINFO status;

    if (!QueryMasterServer(pylon_matchmaking_hostname->GetString(), "/server/byToken", svRequestBody, svResponseBuf, svOutMessage, status))
    {
        return false;
    }

    if (bDebugLog)
    {
        DevMsg(eDLL_T::ENGINE, "%s - Comp-server replied with status: '%d'\n", __FUNCTION__, status);
    }

    try
    {
        if (status == 200) // STATUS_OK
        {
            nlohmann::json jsResultBody = nlohmann::json::parse(svResponseBuf);

            if (bDebugLog)
            {
                string svResultBody = jsResultBody.dump(4);
                DevMsg(eDLL_T::ENGINE, "%s - Comp-server response body:\n%s\n", __FUNCTION__, svResultBody.c_str());
            }

            if (jsResultBody["success"].is_boolean() && jsResultBody["success"])
            {
                slOutServer = NetGameServer_t
                {
                        jsResultBody["server"].value("name",""),
                        jsResultBody["server"].value("description",""),
                        jsResultBody["server"].value("hidden","false") == "true",
                        jsResultBody["server"].value("map",""),
                        jsResultBody["server"].value("playlist",""),
                        jsResultBody["server"].value("ip",""),
                        jsResultBody["server"].value("port", ""),
                        jsResultBody["server"].value("key",""),
                        jsResultBody["server"].value("checksum",""),
                        jsResultBody["server"].value("version", SDK_VERSION),
                        jsResultBody["server"].value("playerCount", ""),
                        jsResultBody["server"].value("maxPlayers", ""),
                        jsResultBody["server"].value("timeStamp", 0),
                        jsResultBody["server"].value("publicRef", ""),
                        jsResultBody["server"].value("cachedId", ""),
                };
                return true;
            }
            else
            {
                ExtractError(jsResultBody, svOutMessage, status);
                return false;
            }
        }
        else
        {
            ExtractError(svResponseBuf, svOutMessage, status, "Server not found");
            return false;
        }
    }
    catch (const std::exception& ex)
    {
        Warning(eDLL_T::ENGINE, "%s - Exception while parsing comp-server response:\n%s\n", __FUNCTION__, ex.what());
    }

    return false;
}

//-----------------------------------------------------------------------------
// Purpose: Sends host server POST request.
// Input  : &svOutMessage - 
//			&svOutToken - 
//			&netGameServer - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CPylon::PostServerHost(string& svOutMessage, string& svOutToken, const NetGameServer_t& netGameServer) const
{
    nlohmann::json jsRequestBody = nlohmann::json::object();
    jsRequestBody["name"] = netGameServer.m_svHostName;
    jsRequestBody["description"] = netGameServer.m_svDescription;
    jsRequestBody["hidden"] = netGameServer.m_bHidden;
    jsRequestBody["map"] = netGameServer.m_svHostMap;
    jsRequestBody["playlist"] = netGameServer.m_svPlaylist;
    jsRequestBody["ip"] = netGameServer.m_svIpAddress;
    jsRequestBody["port"] = netGameServer.m_svGamePort;
    jsRequestBody["key"] = netGameServer.m_svEncryptionKey;
    jsRequestBody["checksum"] = netGameServer.m_svRemoteChecksum;
    jsRequestBody["version"] = netGameServer.m_svSDKVersion;
    jsRequestBody["playerCount"] = netGameServer.m_svPlayerCount;
    jsRequestBody["maxPlayers"] = netGameServer.m_svMaxPlayers;
    jsRequestBody["timeStamp"] = netGameServer.m_nTimeStamp;
    jsRequestBody["publicRef"] = netGameServer.m_svPublicRef;
    jsRequestBody["cachedId"] = netGameServer.m_svCachedId;

    string svRequestBody = jsRequestBody.dump(4);
    string svResponseBuf;

    const bool bDebugLog = pylon_showdebuginfo->GetBool();
    if (bDebugLog)
    {
        DevMsg(eDLL_T::ENGINE, "%s - Sending post host request to comp-server:\n%s\n", __FUNCTION__, svRequestBody.c_str());
    }

    CURLINFO status;
    if (!QueryMasterServer(pylon_matchmaking_hostname->GetString(), "/servers/add", svRequestBody, svResponseBuf, svOutMessage, status))
    {
        return false;
    }

    if (bDebugLog)
    {
        DevMsg(eDLL_T::ENGINE, "%s - Comp-server replied with status: '%d'\n", __FUNCTION__, status);
    }

    try
    {
        if (status == 200) // STATUS_OK
        {
            nlohmann::json jsResultBody = nlohmann::json::parse(svResponseBuf);

            if (bDebugLog)
            {
                string svResultBody = jsResultBody.dump(4);
                DevMsg(eDLL_T::ENGINE, "%s - Comp-server response body:\n%s\n", __FUNCTION__, svResultBody.c_str());
            }

            if (jsResultBody["success"].is_boolean() && jsResultBody["success"].get<bool>())
            {
                if (jsResultBody["token"].is_string())
                {
                    svOutToken = jsResultBody["token"].get<string>();
                }
                else
                {
                    svOutMessage = Format("Invalid response with status: %d", static_cast<int>(status));
                    svOutToken.clear();
                }

                return true;
            }
            else
            {
                ExtractError(jsResultBody, svOutMessage, status);
                svOutToken.clear();

                return false;
            }
        }
        else
        {
            ExtractError(svResponseBuf, svOutMessage, status, "Server host error");
            svOutToken.clear();

            return false;
        }
    }
    catch (const std::exception& ex)
    {
        Warning(eDLL_T::ENGINE, "%s - Exception while parsing comp-server response:\n%s\n", __FUNCTION__, ex.what());
    }

    return false;
}

#ifdef DEDICATED
//-----------------------------------------------------------------------------
// Purpose: Send keep alive request to Pylon Master Server.
// Input  : &netGameServer - 
// Output : Returns true on success, false otherwise.
//-----------------------------------------------------------------------------
bool CPylon::KeepAlive(const NetGameServer_t& netGameServer)
{
    if (!g_pServer->IsActive() || !sv_pylonVisibility->GetBool()) // Check for active game.
    {
        return false;
    }

    string svHostToken;
    string svErrorMsg;

    bool result = PostServerHost(svErrorMsg, svHostToken, netGameServer);
    if (!result)
    {
        if (!svErrorMsg.empty() && m_ErrorMsg.compare(svErrorMsg) != NULL)
        {
            m_ErrorMsg = svErrorMsg;
            Error(eDLL_T::SERVER, NO_ERROR, "%s\n", svErrorMsg.c_str());
        }
    }
    else // Attempt to log the token, if there is one.
    {
        if (!svHostToken.empty() && m_Token.compare(svHostToken) != NULL)
        {
            m_Token = svHostToken;
            DevMsg(eDLL_T::SERVER, "Published server with token: %s'%s%s%s'\n",
                g_svReset.c_str(), g_svGreyB.c_str(),
                svHostToken.c_str(), g_svReset.c_str());
        }
    }

    return result;
}
#endif // DEDICATED

//-----------------------------------------------------------------------------
// Purpose: Checks if client is banned on the comp server.
// Input  : &svIpAddress - 
//			nNucleusID - 
//			&svOutReason - 
// Output : Returns true if banned, false if not banned.
//-----------------------------------------------------------------------------
bool CPylon::CheckForBan(const string& svIpAddress, const uint64_t nNucleusID, string& svOutReason) const
{
    nlohmann::json jsRequestBody = nlohmann::json::object();
    jsRequestBody["id"] = nNucleusID;
    jsRequestBody["ip"] = svIpAddress;

    string svRequestBody = jsRequestBody.dump(4);
    string svResponseBuf;
    string svOutMessage;
    CURLINFO status;

    const bool bDebugLog = pylon_showdebuginfo->GetBool();

    if (bDebugLog)
    {
        DevMsg(eDLL_T::ENGINE, "%s - Sending ban check request to comp-server:\n%s\n", __FUNCTION__, svRequestBody.c_str());
    }

    if (!QueryMasterServer(pylon_matchmaking_hostname->GetString(), "/banlist/isBanned", svRequestBody, svResponseBuf, svOutMessage, status))
    {
        return false;
    }

    if (bDebugLog)
    {
        DevMsg(eDLL_T::ENGINE, "%s - Comp-server replied with status: '%d'\n", __FUNCTION__, status);
    }

    if (status != 200)
    {
        Error(eDLL_T::ENGINE, NO_ERROR, "%s - Failed to query comp-server: status code = %d\n", __FUNCTION__, status);
        return false;
    }

    try
    {
        nlohmann::json jsResultBody = nlohmann::json::parse(svResponseBuf);

        if (bDebugLog)
        {
            string svResultBody = jsResultBody.dump(4);
            DevMsg(eDLL_T::ENGINE, "%s - Comp-server response body:\n%s\n", __FUNCTION__, svResultBody.c_str());
        }

        if (jsResultBody["success"].is_boolean() && jsResultBody["success"].get<bool>())
        {
            if (jsResultBody["banned"].is_boolean() && jsResultBody["banned"].get<bool>())
            {
                svOutReason = jsResultBody.value("reason", "#DISCONNECT_BANNED");
                return true;
            }
        }
    }
    catch (const std::exception& ex)
    {
        Warning(eDLL_T::ENGINE, "%s - Exception while parsing comp-server response:\n%s\n", __FUNCTION__, ex.what());
    }

    return false;
}

//-----------------------------------------------------------------------------
// Purpose: Sends query to master server.
// Input  : &svHostName - 
//			&svApi - 
//			&svInRequest - 
//          &svResponse - 
//          &svOutMessage - 
//          &outStatus - 
// Output : Returns true if successful, false otherwise.
//-----------------------------------------------------------------------------
bool CPylon::QueryMasterServer(const string& svHostName, const string& svApi, const string& svInRequest, 
    string& svOutResponse, string& svOutMessage, CURLINFO& outStatus) const
{
    string svUrl;
    CURLFormatUrl(svUrl, svHostName, svApi);

    curl_slist* sList = nullptr;
    CURL* curl = CURLInitRequest(svUrl, svInRequest, svOutResponse, sList);
    if (!curl)
    {
        return false;
    }

    CURLcode res = CURLSubmitRequest(curl, sList);
    if (!CURLHandleError(curl, res, svOutMessage))
    {
        return false;
    }

    outStatus = CURLRetrieveInfo(curl);
    return true;
}

//-----------------------------------------------------------------------------
// Purpose: Extracts the error from the result body.
// Input  : &resultBody - 
//          &outMessage - 
//          status - 
//          *errorText - 
//-----------------------------------------------------------------------------
void CPylon::ExtractError(const nlohmann::json& resultBody, string& outMessage, CURLINFO status, const char* errorText) const
{
    if (resultBody["error"].is_string())
    {
        outMessage = resultBody["error"].get<string>();
    }
    else
    {
        if (!errorText)
        {
            errorText = "Unknown error with status";
        }

        outMessage = Format("%s: %d", errorText, static_cast<int>(status));
    }
}

//-----------------------------------------------------------------------------
// Purpose: Extracts the error from the response buffer.
// Input  : &resultBody - 
//          &outMessage - 
//          status - 
//          *errorText - 
//-----------------------------------------------------------------------------
void CPylon::ExtractError(const string& responseBuffer, string& outMessage, CURLINFO status, const char* errorText) const
{
    if (!responseBuffer.empty())
    {
        nlohmann::json resultBody = nlohmann::json::parse(responseBuffer);
        ExtractError(resultBody, outMessage, status, errorText);
    }
    else if (status)
    {
        outMessage = Format("Failed comp-server query: %d", static_cast<int>(status));
    }
    else
    {
        outMessage = Format("Failed to reach comp-server: %s", "connection timed-out");
    }
}

///////////////////////////////////////////////////////////////////////////////
CPylon* g_pMasterServer(new CPylon());
