/******************************************************************************
-------------------------------------------------------------------------------
File   : IConsole.cpp
Date   : 18:07:2021
Author : Kawe Mazidjatari
Purpose: Implements the in-game console front-end
-------------------------------------------------------------------------------
History:
- 15:06:2021 | 14:56 : Created by Kawe Mazidjatari
- 07:08:2021 | 15:22 : Multithread 'CommandExecute' operations to prevent deadlock in render thread
- 07:08:2021 | 15:25 : Fix a race condition that occured when detaching the 'CommandExecute' thread

******************************************************************************/

#include "core/stdafx.h"
#include "core/init.h"
#include "core/resource.h"
#include "tier0/commandline.h"
#include "tier1/cvar.h"
#include "windows/id3dx.h"
#include "windows/console.h"
#include "windows/resource.h"
#include "gameui/IConsole.h"
#include "client/vengineclient_impl.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CConsole::CConsole(void)
{
    ClearLog();
    memset(m_szInputBuf, '\0', sizeof(m_szInputBuf));

    m_nHistoryPos     = -1;
    m_bAutoScroll     = true;
    m_bScrollToBottom = false;
    m_bInitialized    = false;
    m_pszConsoleTitle = "Console";

    m_vsvCommands.push_back("CLEAR");
    m_vsvCommands.push_back("HELP");
    m_vsvCommands.push_back("HISTORY");

    snprintf(m_szSummary, 256, "%llu history items", m_vsvHistory.size());

    std::thread think(&CConsole::Think, this);
    think.detach();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CConsole::~CConsole(void)
{
    ClearLog();
    m_vsvHistory.clear();
}

//-----------------------------------------------------------------------------
// Purpose: game console setup
// Output : true on success, false otherwise
//-----------------------------------------------------------------------------
bool CConsole::Setup(void)
{
    SetStyleVar();

    int k = 0; // Get all image resources for displaying flags.
    for (int i = IDB_PNG3; i <= IDB_PNG18; i++)
    {
        m_vFlagIcons.push_back(MODULERESOURCE());
        m_vFlagIcons[k] = GetModuleResource(i);

        bool ret = LoadTextureBuffer(reinterpret_cast<unsigned char*>(m_vFlagIcons[k].m_pData), static_cast<int>(m_vFlagIcons[k].m_nSize),
            &m_vFlagIcons[k].m_idIcon, &m_vFlagIcons[k].m_nWidth, &m_vFlagIcons[k].m_nHeight);
        if (!ret)
        {
            IM_ASSERT(ret);
            return false;
        }
        k++;
    }
    return true;
}

//-----------------------------------------------------------------------------
// Purpose: game console main render loop
//-----------------------------------------------------------------------------
void CConsole::Draw(void)
{
    if (!m_bInitialized)
    {
        Setup();
        m_bInitialized = true;
    }

    {
        //ImGui::ShowStyleEditor();
        //ImGui::ShowDemoWindow();
    }

    /**************************
     * BASE PANEL SETUP       *
     **************************/
    {
        int nVars{};
        if (!m_bActivate)
        {
            return;
        }
        if (m_bDefaultTheme)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 8.f, 10.f }); nVars++;
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_flFadeAlpha);               nVars++;
        }
        else
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 4.f, 6.f });  nVars++;
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);              nVars++;
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_flFadeAlpha);               nVars++;
        }

        ImGui::SetNextWindowSize(ImVec2(1000, 600), ImGuiCond_FirstUseEver);
        ImGui::SetWindowPos(ImVec2(-1000, 50), ImGuiCond_FirstUseEver);

        BasePanel();

        ImGui::PopStyleVar(nVars);
    }

    /**************************
     * SUGGESTION PANEL SETUP *
     **************************/
    {
        int nVars{};
        if (CanAutoComplete())
        {
            if (m_bDefaultTheme)
            {
                static ImGuiStyle& style = ImGui::GetStyle();
                m_ivSuggestWindowPos.y = m_ivSuggestWindowPos.y + style.WindowPadding.y + 1.5f;
            }

            ImGui::SetNextWindowPos(m_ivSuggestWindowPos);
            ImGui::SetNextWindowSize(m_ivSuggestWindowSize);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(500, 37)); nVars++;
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);         nVars++;
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_flFadeAlpha);           nVars++;

            SuggestPanel();

            ImGui::PopStyleVar(nVars);
        }
    }
}

//-----------------------------------------------------------------------------
// Purpose: runs tasks for the console while not being drawn
//-----------------------------------------------------------------------------
void CConsole::Think(void)
{
    for (;;) // Loop running at 100-tps.
    {
        if (m_ivConLog.size() > con_max_size_logvector->GetSizeT())
        {
            while (m_ivConLog.size() > con_max_size_logvector->GetSizeT() / 4 * 3)
            {
                m_ivConLog.erase(m_ivConLog.begin());
                m_nScrollBack++;
            }
        }

        while (static_cast<int>(m_vsvHistory.size()) > 512)
        {
            m_vsvHistory.erase(m_vsvHistory.begin());
        }

        if (m_bActivate)
        {
            if (m_flFadeAlpha <= 1.f)
            {
                m_flFadeAlpha += 0.05;
            }
        }
        else // Reset to full transparent.
        {
            m_flFadeAlpha = 0.f;
            m_bReclaimFocus = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

//-----------------------------------------------------------------------------
// Purpose: draws the console's main surface
// Input  : *bDraw - 
//-----------------------------------------------------------------------------
void CConsole::BasePanel(void)
{
    if (!ImGui::Begin(m_pszConsoleTitle, &m_bActivate))
    {
        ImGui::End();
        return;
    }

    // Reserve enough left-over height and width for 1 separator + 1 input text
    const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    const float footer_width_to_reserve  = ImGui::GetStyle().ItemSpacing.y + ImGui::GetWindowWidth();

    ///////////////////////////////////////////////////////////////////////
    ImGui::Separator();
    if (ImGui::BeginPopup("Options"))
    {
        OptionsPanel();
    }
    if (ImGui::Button("Options"))
    {
        ImGui::OpenPopup("Options");
    }

    ImGui::SameLine();
    m_itFilter.Draw("Filter | ", footer_width_to_reserve - 500);

    ImGui::SameLine();
    ImGui::Text(m_szSummary);

    ImGui::Separator();

    ///////////////////////////////////////////////////////////////////////
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

    if (m_bCopyToClipBoard) { ImGui::LogToClipboard(); }
    ColorLog();
    if (m_bCopyToClipBoard)
    {
        ImGui::LogToClipboard();
        ImGui::LogFinish();

        m_bCopyToClipBoard = false;
    }

    if (m_nScrollBack > 0)
    {
        ImGui::SetScrollY(ImGui::GetScrollY() - m_nScrollBack * ImGui::GetTextLineHeightWithSpacing() - m_nScrollBack - 90);
        m_nScrollBack = 0;
    }

    if (m_bScrollToBottom || (m_bAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
    {
        ImGui::SetScrollHereY(1.0f);
        m_bScrollToBottom = false;
    }

    ///////////////////////////////////////////////////////////////////////
    ImGui::EndChild();
    ImGui::Separator();

    ImGui::PushItemWidth(footer_width_to_reserve - 80);
    if (ImGui::InputText("##input", m_szInputBuf, IM_ARRAYSIZE(m_szInputBuf), m_nInputFlags, &TextEditCallbackStub, reinterpret_cast<void*>(this)))
    {
        if (m_nSuggestPos != -1)
        {
            // Remove the default value from ConVar before assigning it to the input buffer.
            string svConVar = m_vsvSuggest[m_nSuggestPos].m_svName.substr(0, m_vsvSuggest[m_nSuggestPos].m_svName.find(' ')) + " ";
            memmove(m_szInputBuf, svConVar.c_str(), svConVar.size() + 1);

            ResetAutoComplete();
        }
        else
        {
            if (m_szInputBuf[0])
            {
                ProcessCommand(m_szInputBuf);
                memset(m_szInputBuf, '\0', 1);
            }

            ResetAutoComplete();
        }
    }

    // Auto-focus on window apparition.
    ImGui::SetItemDefaultFocus();

    // Auto-focus previous widget.
    if (m_bReclaimFocus)
    {
        ImGui::SetKeyboardFocusHere(-1);
        m_bReclaimFocus = false;
    }

    int nPad = 0;
    if (static_cast<int>(m_vsvSuggest.size()) > 1)
    {
        // Pad with 18 to keep all items in view.
        nPad = 18;
    }
    m_ivSuggestWindowPos = ImGui::GetItemRectMin();
    m_ivSuggestWindowPos.y += ImGui::GetItemRectSize().y;
    m_ivSuggestWindowSize = ImVec2(600, nPad + std::clamp(static_cast<float>(m_vsvSuggest.size()) * 13.0f, 37.0f, 127.5f));

    ImGui::SameLine();
    if (ImGui::Button("Submit"))
    {
        if (m_szInputBuf[0])
        {
            ProcessCommand(m_szInputBuf);
            memset(m_szInputBuf, '\0', 1);
        }
        ResetAutoComplete();
    }
    ImGui::End();
}

//-----------------------------------------------------------------------------
// Purpose: draws the options panel
//-----------------------------------------------------------------------------
void CConsole::OptionsPanel(void)
{
    ImGui::Checkbox("Auto-Scroll", &m_bAutoScroll);

    ImGui::SameLine();
    ImGui::PushItemWidth(100);

    ImGui::PopItemWidth();

    if (ImGui::SmallButton("Clear"))
    {
        ClearLog();
    }

    ImGui::SameLine();
    m_bCopyToClipBoard = ImGui::SmallButton("Copy");

    ImGui::Text("Console Hotkey:");
    ImGui::SameLine();

    if (ImGui::Hotkey("##OpenIConsoleBind0", &g_pImGuiConfig->IConsole_Config.m_nBind0, ImVec2(80, 80)))
    {
        g_pImGuiConfig->Save();
    }

    ImGui::Text("Browser Hotkey:");
    ImGui::SameLine();

    if (ImGui::Hotkey("##OpenIBrowserBind0", &g_pImGuiConfig->IBrowser_Config.m_nBind0, ImVec2(80, 80)))
    {
        g_pImGuiConfig->Save();
    }

    ImGui::EndPopup();
}

//-----------------------------------------------------------------------------
// Purpose: draws the suggestion panel with results based on user input
//-----------------------------------------------------------------------------
void CConsole::SuggestPanel(void)
{
    ImGui::Begin("##suggest", nullptr, m_nSuggestFlags);
    ImGui::PushAllowKeyboardFocus(false);

    for (size_t i = 0; i < m_vsvSuggest.size(); i++)
    {
        bool bIsIndexActive = m_nSuggestPos == i;
        ImGui::PushID(i);

        if (con_suggestion_showflags->GetBool())
        {
            int k = ColorCodeFlags(m_vsvSuggest[i].m_nFlags);
            ImGui::Image(m_vFlagIcons[k].m_idIcon, ImVec2(m_vFlagIcons[k].m_nWidth, m_vFlagIcons[k].m_nHeight));
            ImGui::SameLine();
        }

        if (ImGui::Selectable(m_vsvSuggest[i].m_svName.c_str(), bIsIndexActive))
        {
            ImGui::Separator();

            // Remove the default value from ConVar before assigning it to the input buffer.
            string svConVar = m_vsvSuggest[i].m_svName.substr(0, m_vsvSuggest[i].m_svName.find(' ')) + " ";
            memmove(m_szInputBuf, svConVar.c_str(), svConVar.size() + 1);

            ResetAutoComplete();
        }
        ImGui::PopID();

        // Make sure we bring the currently 'active' item into view.
        if (m_bSuggestMoved && bIsIndexActive)
        {
            ImGuiWindow* pWindow = ImGui::GetCurrentWindow();
            ImRect imRect = ImGui::GetCurrentContext()->LastItemData.Rect;

            // Reset to keep flag in display.
            imRect.Min.x = pWindow->InnerRect.Min.x;
            imRect.Max.x = pWindow->InnerRect.Min.x; // Set to Min.x on purpose!

            // Eliminate jiggle when going up/down in the menu.
            imRect.Min.y += 1;
            imRect.Max.y -= 1;

            ImGui::ScrollToRect(pWindow, imRect);
            m_bSuggestMoved = false;
        }

        if (m_bSuggestUpdate)
        {
            ImGui::SetScrollHereY(0.f);
            m_bSuggestUpdate = false;
        }
    }

    ImGui::PopAllowKeyboardFocus();
    ImGui::End();
}

//-----------------------------------------------------------------------------
// Purpose: checks if the console can autocomplete based on input
// Output : true to perform autocomplete, false otherwise
//-----------------------------------------------------------------------------
bool CConsole::CanAutoComplete(void)
{
    // Show ConVar/ConCommand suggestions when at least 2 characters have been entered.
    if (strlen(m_szInputBuf) > 1)
    {
        static char szCurInputBuf[512]{};
        if (strcmp(m_szInputBuf, szCurInputBuf) != 0) // Update suggestions if input buffer changed.
        {
            memmove(szCurInputBuf, m_szInputBuf, strlen(m_szInputBuf) + 1);
            FindFromPartial();
        }
        if (static_cast<int>(m_vsvSuggest.size()) <= 0)
        {
            m_nSuggestPos = -1;
            return false;
        }
    }
    else
    {
        m_nSuggestPos = -1;
        return false;
    }

    // Don't suggest if user tries to assign value to ConVar or execute ConCommand.
    if (strstr(m_szInputBuf, " ") || strstr(m_szInputBuf, ";"))
    {
        ResetAutoComplete();
        return false;
    }
    m_bSuggestActive = true;
    return true;
}

//-----------------------------------------------------------------------------
// Purpose: resets the autocomplete window
//-----------------------------------------------------------------------------
void CConsole::ResetAutoComplete(void)
{
    m_bSuggestActive = false;
    m_nSuggestPos = -1;
    m_bReclaimFocus = true;
}

//-----------------------------------------------------------------------------
// Purpose: find ConVars/ConCommands from user input and add to vector
//-----------------------------------------------------------------------------
void CConsole::FindFromPartial(void)
{
    m_nSuggestPos = -1;
    m_bSuggestUpdate = true;
    m_vsvSuggest.clear();

    for (size_t i = 0; i < m_vsvCommandBases.size(); i++)
    {
        if (m_vsvSuggest.size() < con_suggestion_limit->GetInt())
        {
            if (m_vsvCommandBases[i].m_svName.find(m_szInputBuf) != string::npos)
            {
                if (std::find(m_vsvSuggest.begin(), m_vsvSuggest.end(), 
                    m_vsvCommandBases[i].m_svName) == m_vsvSuggest.end())
                {
                    int nFlags{};
                    string svValue;
                    ConCommandBase* pCommandBase = g_pCVar->FindCommandBase(m_vsvCommandBases[i].m_svName.c_str());

                    if (pCommandBase)
                    {
                        if (!pCommandBase->IsCommand())
                        {
                            ConVar* pConVar = reinterpret_cast<ConVar*>(pCommandBase);

                            svValue = " = ["; // Assign default value to string if its a ConVar.
                            svValue.append(pConVar->GetString());
                            svValue.append("]");
                        }
                        if (con_suggestion_showhelptext->GetBool())
                        {
                            if (pCommandBase->GetHelpText())
                            {
                                string svHelpText = pCommandBase->GetHelpText();
                                if (!svHelpText.empty())
                                {
                                    svValue.append(" - \"" + svHelpText + "\"");
                                }
                            }
                            if (pCommandBase->GetUsageText())
                            {
                                string svUsageText = pCommandBase->GetUsageText();
                                if (!svUsageText.empty())
                                {
                                    svValue.append(" - \"" + svUsageText + "\"");
                                }
                            }
                        }
                        if (con_suggestion_showflags->GetBool())
                        {
                            if (con_suggestion_flags_realtime->GetBool())
                            {
                                nFlags = pCommandBase->GetFlags();
                            }
                            else // Display compile-time flags instead.
                            {
                                nFlags = m_vsvCommandBases[i].m_nFlags;
                            }
                        }
                    }
                    m_vsvSuggest.push_back(CSuggest(m_vsvCommandBases[i].m_svName + svValue, nFlags));
                }
            }
        }
        else { break; }
    }
    std::sort(m_vsvSuggest.begin(), m_vsvSuggest.end());
}

//-----------------------------------------------------------------------------
// Purpose: executes submitted commands in a separate thread
// Input  : pszCommand - 
//-----------------------------------------------------------------------------
void CConsole::ProcessCommand(const char* pszCommand)
{
    AddLog(ImVec4(1.00f, 0.80f, 0.60f, 1.00f), "# %s\n", PrintPercentageEscape(pszCommand).c_str());

    std::thread t(CEngineClient_CommandExecute, this, pszCommand);
    t.detach(); // Detach from render thread.

    // This is to avoid a race condition.
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    m_nHistoryPos = -1;
    for (int i = static_cast<int>(m_vsvHistory.size()) - 1; i >= 0; i--)
    {
        if (Stricmp(m_vsvHistory[i].c_str(), pszCommand) == 0)
        {
            m_vsvHistory.erase(m_vsvHistory.begin() + i);
            break;
        }
    }

    m_vsvHistory.push_back(Strdup(pszCommand));
    if (Stricmp(pszCommand, "CLEAR") == 0)
    {
        ClearLog();
    }
    else if (Stricmp(pszCommand, "HELP") == 0)
    {
        AddLog(ImVec4(0.81f, 0.81f, 0.81f, 1.00f), "Commands:");
        for (int i = 0; i < static_cast<int>(m_vsvCommands.size()); i++)
        {
            AddLog(ImVec4(0.81f, 0.81f, 0.81f, 1.00f), "- %s", m_vsvCommands[i].c_str());
        }

        AddLog(ImVec4(0.81f, 0.81f, 0.81f, 1.00f), "Log types:");
        AddLog(ImVec4(0.59f, 0.58f, 0.73f, 1.00f), "Script(S): = Server DLL (Script)");
        AddLog(ImVec4(0.59f, 0.58f, 0.63f, 1.00f), "Script(C): = Client DLL (Script)");
        AddLog(ImVec4(0.59f, 0.48f, 0.53f, 1.00f), "Script(U): = UI DLL (Script)");

        AddLog(ImVec4(0.23f, 0.47f, 0.85f, 1.00f), "Native(S): = Server DLL (Code)");
        AddLog(ImVec4(0.46f, 0.46f, 0.46f, 1.00f), "Native(C): = Client DLL (Code)");
        AddLog(ImVec4(0.59f, 0.35f, 0.46f, 1.00f), "Native(U): = UI DLL (Code)");

        AddLog(ImVec4(0.70f, 0.70f, 0.70f, 1.00f), "Native(E): = Engine DLL (Code)");
        AddLog(ImVec4(0.32f, 0.64f, 0.72f, 1.00f), "Native(F): = FileSys DLL (Code)");
        AddLog(ImVec4(0.36f, 0.70f, 0.35f, 1.00f), "Native(R): = RTech DLL (Code)");
        AddLog(ImVec4(0.75f, 0.41f, 0.67f, 1.00f), "Native(M): = MatSys DLL (Code)");
    }
    else if (Stricmp(pszCommand, "HISTORY") == 0)
    {
        int nFirst = static_cast<int>(m_vsvHistory.size()) - 10;
        for (int i = nFirst > 0 ? nFirst : 0; i < static_cast<int>(m_vsvHistory.size()); i++)
        {
            AddLog(ImVec4(0.81f, 0.81f, 0.81f, 1.00f), "%3d: %s\n", i, m_vsvHistory[i].c_str());
        }
    }

    m_bScrollToBottom = true;
}

//-----------------------------------------------------------------------------
// Purpose: returns flag image index for CommandBase (must be aligned with resource.h!)
// Input  : nFlags - 
//-----------------------------------------------------------------------------
int CConsole::ColorCodeFlags(int nFlags) const
{
    switch (nFlags)
    {
    case FCVAR_NONE:
        return 1;
    case FCVAR_DEVELOPMENTONLY:
        return 2;
    case FCVAR_GAMEDLL:
        return 3;
    case FCVAR_CLIENTDLL:
        return 4;
    case FCVAR_CHEAT:
        return 5;
    case FCVAR_RELEASE:
        return 6;
    case FCVAR_DEVELOPMENTONLY | FCVAR_GAMEDLL:
        return 7;
    case FCVAR_DEVELOPMENTONLY | FCVAR_CLIENTDLL:
        return 8;
    case FCVAR_DEVELOPMENTONLY | FCVAR_HIDDEN:
        return 9;
    case FCVAR_DEVELOPMENTONLY | FCVAR_REPLICATED:
        return 10;
    case FCVAR_DEVELOPMENTONLY | FCVAR_CHEAT:
        return 11;
    case FCVAR_REPLICATED | FCVAR_CHEAT:
        return 12;
    case FCVAR_REPLICATED | FCVAR_RELEASE:
        return 13;
    case FCVAR_GAMEDLL | FCVAR_CHEAT:
        return 14;
    case FCVAR_CLIENTDLL | FCVAR_CHEAT:
        return 15;
    default:
        return 0;
    }
}

//-----------------------------------------------------------------------------
// Purpose: console input box callback
// Input  : *iData - 
// Output : 
//-----------------------------------------------------------------------------
int CConsole::TextEditCallback(ImGuiInputTextCallbackData* iData)
{
    switch (iData->EventFlag)
    {
    case ImGuiInputTextFlags_CallbackCompletion:
    {
        // Locate beginning of current word.
        const char* pszWordEnd = iData->Buf + iData->CursorPos;
        const char* pszWordStart = pszWordEnd;
        while (pszWordStart > iData->Buf)
        {
            const char c = pszWordStart[-1];
            if (c == ' ' || c == '\t' || c == ',' || c == ';')
            {
                break;
            }
            pszWordStart--;
        }
        break;
    }
    case ImGuiInputTextFlags_CallbackHistory:
    {
        if (m_bSuggestActive)
        {
            if (iData->EventKey == ImGuiKey_UpArrow && m_nSuggestPos > - 1)
            {
                m_nSuggestPos--;
                m_bSuggestMoved = true;
            }
            else if (iData->EventKey == ImGuiKey_DownArrow)
            {
                if (m_nSuggestPos < static_cast<int>(m_vsvSuggest.size()) - 1)
                {
                    m_nSuggestPos++;
                    m_bSuggestMoved = true;
                }
            }
        }
        else // Allow user to navigate through the history if suggest isn't drawn.
        {
            const int nPrevHistoryPos = m_nHistoryPos;
            if (iData->EventKey == ImGuiKey_UpArrow)
            {
                if (m_nHistoryPos == -1)
                {
                    m_nHistoryPos = static_cast<int>(m_vsvHistory.size()) - 1;
                }
                else if (m_nHistoryPos > 0)
                {
                    m_nHistoryPos--;
                }
            }
            else if (iData->EventKey == ImGuiKey_DownArrow)
            {
                if (m_nHistoryPos != -1)
                {
                    if (++m_nHistoryPos >= static_cast<int>(m_vsvHistory.size()))
                    {
                        m_nHistoryPos = -1;
                    }
                }
            }
            if (nPrevHistoryPos != m_nHistoryPos)
            {
                string svHistory = (m_nHistoryPos >= 0) ? m_vsvHistory[m_nHistoryPos] : "";

                if (!svHistory.empty())
                {
                    if (!strstr(m_vsvHistory[m_nHistoryPos].c_str(), " "))
                    {
                        // Append whitespace to previous entered command if absent or no parameters where passed.
                        svHistory.append(" ");
                    }
                }

                iData->DeleteChars(0, iData->BufTextLen);
                iData->InsertChars(0, svHistory.c_str());
            }
        }
        break;
    }
    case ImGuiInputTextFlags_CallbackAlways:
    {
        static char szCurInputBuf[512]{};
        if (strcmp(m_szInputBuf, szCurInputBuf) != 0) // Only run if changed.
        {
            char szValue[512]{};
            memmove(szCurInputBuf, m_szInputBuf, strlen(m_szInputBuf) + 1);
            sprintf_s(szValue, sizeof(szValue), "%s", m_szInputBuf);

            // Remove space or semicolon before we call 'g_pCVar->FindVar(..)'.
            for (int i = 0; i < strlen(szValue); i++)
            {
                if (szValue[i] == ' ' || szValue[i] == ';')
                {
                    szValue[i] = '\0';
                }
            }

            ConVar* pConVar = g_pCVar->FindVar(szValue);
            if (pConVar != nullptr)
            {
                // Display the current and default value of ConVar if found.
                snprintf(m_szSummary, 256, "(\"%s\", default \"%s\")", pConVar->GetString(), pConVar->GetDefault());
            }
            else
            {
                // Display amount of history items if ConVar cannot be found.
                snprintf(m_szSummary, 256, "%llu history items", m_vsvHistory.size());
            }
            break;
        }
    }
    }
    return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: console input box callback stub
// Input  : *iData - 
// Output : 
//-----------------------------------------------------------------------------
int CConsole::TextEditCallbackStub(ImGuiInputTextCallbackData* iData)
{
    CConsole* pConsole = reinterpret_cast<CConsole*>(iData->UserData);
    return pConsole->TextEditCallback(iData);
}

//-----------------------------------------------------------------------------
// Purpose: adds logs to the vector
// Input  : *fmt - 
//          ... - 
//-----------------------------------------------------------------------------
void CConsole::AddLog(ImVec4 color, const char* fmt, ...) IM_FMTARGS(2)
{
    char buf[1024];
    va_list args{};
    va_start(args, fmt);
    vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
    buf[IM_ARRAYSIZE(buf) - 1] = 0;
    va_end(args);
    m_ivConLog.push_back(CConLog(Strdup(buf), color));
}

//-----------------------------------------------------------------------------
// Purpose: clears the entire vector
//-----------------------------------------------------------------------------
void CConsole::ClearLog(void)
{
    //for (int i = 0; i < m_ivConLog.size(); i++) { free(m_ivConLog[i]); }
    m_ivConLog.clear();
}

//-----------------------------------------------------------------------------
// Purpose: colors important logs
//-----------------------------------------------------------------------------
void CConsole::ColorLog(void) const
{
    for (int i = 0; i < m_ivConLog.size(); i++)
    {
        if (!m_itFilter.PassFilter(m_ivConLog[i].m_svConLog.c_str()))
        {
            continue;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, m_ivConLog[i].m_imColor);
        ImGui::TextWrapped(m_ivConLog[i].m_svConLog.c_str());
        ImGui::PopStyleColor();
        ///////////////////////////////////////////////////////////////////////
    }
}

//-----------------------------------------------------------------------------
// Purpose: sets the console front-end style
//-----------------------------------------------------------------------------
void CConsole::SetStyleVar(void)
{
    ImGuiStyle& style                     = ImGui::GetStyle();
    ImVec4* colors                        = style.Colors;

    if (!CommandLine()->CheckParm("-imgui_default_theme"))
    {
        colors[ImGuiCol_Text]                 = ImVec4(0.81f, 0.81f, 0.81f, 1.00f);
        colors[ImGuiCol_TextDisabled]         = ImVec4(0.56f, 0.56f, 0.56f, 1.00f);
        colors[ImGuiCol_WindowBg]             = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
        colors[ImGuiCol_ChildBg]              = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_PopupBg]              = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
        colors[ImGuiCol_Border]               = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_BorderShadow]         = ImVec4(0.04f, 0.04f, 0.04f, 0.64f);
        colors[ImGuiCol_FrameBg]              = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
        colors[ImGuiCol_FrameBgActive]        = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
        colors[ImGuiCol_TitleBg]              = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
        colors[ImGuiCol_TitleBgActive]        = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
        colors[ImGuiCol_MenuBarBg]            = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.53f, 0.53f, 0.53f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.63f, 0.63f, 0.63f, 1.00f);
        colors[ImGuiCol_CheckMark]            = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_SliderGrab]           = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.53f, 0.53f, 0.53f, 1.00f);
        colors[ImGuiCol_Button]               = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
        colors[ImGuiCol_ButtonHovered]        = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
        colors[ImGuiCol_ButtonActive]         = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
        colors[ImGuiCol_Header]               = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
        colors[ImGuiCol_HeaderHovered]        = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
        colors[ImGuiCol_HeaderActive]         = ImVec4(0.53f, 0.53f, 0.53f, 1.00f);
        colors[ImGuiCol_Separator]            = ImVec4(0.53f, 0.53f, 0.57f, 1.00f);
        colors[ImGuiCol_SeparatorHovered]     = ImVec4(0.53f, 0.53f, 0.53f, 1.00f);
        colors[ImGuiCol_SeparatorActive]      = ImVec4(0.63f, 0.63f, 0.63f, 1.00f);
        colors[ImGuiCol_ResizeGrip]           = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
        colors[ImGuiCol_ResizeGripHovered]    = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
        colors[ImGuiCol_ResizeGripActive]     = ImVec4(0.63f, 0.63f, 0.63f, 1.00f);
        colors[ImGuiCol_Tab]                  = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
        colors[ImGuiCol_TabHovered]           = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
        colors[ImGuiCol_TabActive]            = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);

        style.WindowBorderSize  = 0.0f;
        style.FrameBorderSize   = 1.0f;
        style.ChildBorderSize   = 1.0f;
        style.PopupBorderSize   = 1.0f;
        style.TabBorderSize     = 1.0f;

        style.WindowRounding    = 4.0f;
        style.FrameRounding     = 1.0f;
        style.ChildRounding     = 1.0f;
        style.PopupRounding     = 3.0f;
        style.TabRounding       = 1.0f;
        style.ScrollbarRounding = 1.0f;
    }
    else
    {
        colors[ImGuiCol_WindowBg]               = ImVec4(0.11f, 0.13f, 0.17f, 1.00f);
        colors[ImGuiCol_ChildBg]                = ImVec4(0.02f, 0.04f, 0.06f, 1.00f);
        colors[ImGuiCol_PopupBg]                = ImVec4(0.11f, 0.13f, 0.17f, 1.00f);
        colors[ImGuiCol_Border]                 = ImVec4(0.41f, 0.41f, 0.41f, 0.50f);
        colors[ImGuiCol_BorderShadow]           = ImVec4(0.04f, 0.04f, 0.04f, 0.00f);
        colors[ImGuiCol_FrameBg]                = ImVec4(0.02f, 0.04f, 0.06f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.04f, 0.06f, 0.10f, 1.00f);
        colors[ImGuiCol_FrameBgActive]          = ImVec4(0.04f, 0.07f, 0.12f, 1.00f);
        colors[ImGuiCol_TitleBg]                = ImVec4(0.26f, 0.51f, 0.78f, 1.00f);
        colors[ImGuiCol_TitleBgActive]          = ImVec4(0.26f, 0.51f, 0.78f, 1.00f);
        colors[ImGuiCol_MenuBarBg]              = ImVec4(0.11f, 0.13f, 0.17f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.14f, 0.19f, 0.24f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.23f, 0.36f, 0.51f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.30f, 0.46f, 0.65f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.31f, 0.49f, 0.69f, 1.00f);
        colors[ImGuiCol_SliderGrab]             = ImVec4(0.31f, 0.43f, 0.43f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.41f, 0.56f, 0.57f, 1.00f);
        colors[ImGuiCol_Button]                 = ImVec4(0.31f, 0.43f, 0.43f, 1.00f);
        colors[ImGuiCol_ButtonHovered]          = ImVec4(0.38f, 0.52f, 0.53f, 1.00f);
        colors[ImGuiCol_ButtonActive]           = ImVec4(0.41f, 0.56f, 0.57f, 1.00f);
        colors[ImGuiCol_Header]                 = ImVec4(0.31f, 0.43f, 0.43f, 1.00f);
        colors[ImGuiCol_HeaderHovered]          = ImVec4(0.38f, 0.53f, 0.53f, 1.00f);
        colors[ImGuiCol_HeaderActive]           = ImVec4(0.41f, 0.56f, 0.57f, 1.00f);
        colors[ImGuiCol_Separator]              = ImVec4(0.53f, 0.53f, 0.57f, 0.00f);
        colors[ImGuiCol_ResizeGrip]             = ImVec4(0.41f, 0.41f, 0.41f, 0.50f);
        colors[ImGuiCol_Tab]                    = ImVec4(0.31f, 0.43f, 0.43f, 1.00f);
        colors[ImGuiCol_TabHovered]             = ImVec4(0.38f, 0.53f, 0.53f, 1.00f);
        colors[ImGuiCol_TabActive]              = ImVec4(0.41f, 0.56f, 0.57f, 1.00f);
        colors[ImGuiCol_TableHeaderBg]          = ImVec4(0.14f, 0.19f, 0.24f, 1.00f);
        colors[ImGuiCol_TableBorderStrong]      = ImVec4(0.20f, 0.26f, 0.33f, 1.00f);
        colors[ImGuiCol_TableBorderLight]       = ImVec4(0.22f, 0.29f, 0.37f, 1.00f);

        style.WindowBorderSize  = 1.0f;
        style.FrameBorderSize   = 0.0f;
        style.ChildBorderSize   = 0.0f;
        style.PopupBorderSize   = 1.0f;
        style.TabBorderSize     = 1.0f;

        style.WindowRounding    = 4.0f;
        style.FrameRounding     = 1.0f;
        style.ChildRounding     = 1.0f;
        style.PopupRounding     = 3.0f;
        style.TabRounding       = 1.0f;
        style.ScrollbarRounding = 3.0f;

        m_bDefaultTheme = true;
    }

    style.ItemSpacing       = ImVec2(4, 4);
    style.FramePadding      = ImVec2(4, 4);
    style.WindowPadding     = ImVec2(5, 5);
    style.WindowMinSize = ImVec2(618, 518);
}

CConsole* g_pConsole = new CConsole();
