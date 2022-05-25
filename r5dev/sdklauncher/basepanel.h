#pragma once
#include "sdklauncher_const.h"

class CUIBaseSurface : public Forms::Form
{
public:
	CUIBaseSurface();
	virtual ~CUIBaseSurface() = default;

private:
	void Init();
	void Setup();
	void ParseMaps();
	void ParsePlaylists();


	static void LaunchGame(Forms::Control* pSender);
	static void ReloadPlaylists(Forms::Control* pSender);
	eLaunchMode BuildParameter(string& svParameter);


	enum class eMode
	{
		NONE = -1,
		HOST,
		SERVER,
		CLIENT,
	};
	enum class eVisibility
	{
		PUBLIC,
		HIDDEN,
	};

	UIX::UIXTextBox* m_WidthTextBox;
	UIX::UIXTextBox* m_HeightTextBox;
	UIX::UIXTextBox* m_WorkerThreadsTextBox;
	UIX::UIXTextBox* m_ReservedCoresTextBox;
	UIX::UIXTextBox* m_FpsTextBox;
	UIX::UIXTextBox* m_PlaylistFileTextBox;
	UIX::UIXTextBox* m_HostNameTextBox;
	UIX::UIXTextBox* m_LaunchArgsTextBox;
	// Labels
	UIX::UIXLabel* m_WorkerThreadsLabel;
	UIX::UIXLabel* m_ReservedCoresLabel;
	UIX::UIXLabel* m_MapLabel;
	UIX::UIXLabel* m_PlaylistLabel;
	UIX::UIXLabel* m_ModeLabel;
	UIX::UIXLabel* m_FpsLabel;
	UIX::UIXLabel* m_ResolutionLabel;
	UIX::UIXLabel* m_PlaylistFileLabel;
	UIX::UIXLabel* m_HostNameLabel;
	UIX::UIXLabel* m_VisibilityLabel;
	UIX::UIXLabel* m_LaunchArgsLabel;
	// Boxes
	UIX::UIXGroupBox* m_GameGroup;
	UIX::UIXGroupBox* m_MainGroup;
	UIX::UIXGroupBox* m_GameGroupExt;
	UIX::UIXGroupBox* m_MainGroupExt;
	UIX::UIXGroupBox* m_ConsoleGroup;
	UIX::UIXGroupBox* m_EngineBaseGroup;
	UIX::UIXGroupBox* m_EngineNetworkGroup;
	UIX::UIXGroupBox* m_EngineVideoGroup;
	// Toggles
	UIX::UIXCheckBox* m_CheatsToggle;
	UIX::UIXCheckBox* m_DevelopmentToggle;
	UIX::UIXCheckBox* m_ConsoleToggle;
	UIX::UIXCheckBox* m_WindowedToggle;
	UIX::UIXCheckBox* m_NoBorderToggle;
	UIX::UIXCheckBox* m_SingleCoreDediToggle;
	UIX::UIXCheckBox* m_NoAsyncJobsToggle;
	UIX::UIXCheckBox* m_NetEncryptionToggle;
	UIX::UIXCheckBox* m_NetRandomKeyToggle;
	UIX::UIXCheckBox* m_NoQueuedPacketThread;
	UIX::UIXCheckBox* m_NoTimeOutToggle;
	UIX::UIXCheckBox* m_ColorConsoleToggle;
	// Combo
	UIX::UIXComboBox* m_MapCombo;
	UIX::UIXComboBox* m_PlaylistCombo;
	UIX::UIXComboBox* m_ModeCombo;
	UIX::UIXComboBox* m_VisibilityCombo;
	// Buttons
	UIX::UIXButton* m_CleanSDK;
	UIX::UIXButton* m_UpdateSDK;
	UIX::UIXButton* m_LaunchSDK;

	UIX::UIXListView* m_ConsoleListView;
};
