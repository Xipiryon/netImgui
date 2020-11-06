//=================================================================================================
// SAMPLE 
//-------------------------------------------------------------------------------------------------
// Common code shared by all samples 
//=================================================================================================

#include <NetImgui_Api.h>
#include "..\Common\Sample.h"
#include "..\Common\WarningDisable.h"

namespace SampleClient
{

static int sClientPort				= NetImgui::kDefaultClientPort;
static int sServerPort				= NetImgui::kDefaultServerPort;
static char sServerHostname[128]	= {"localhost"};
static bool sbShowDemoWindow		= false;
static bool sbShowConnectToWindow	= false;
static bool sbShowWaitForWindow		= false;

bool DisplayConnectedStatus()
{
	//-----------------------------------------------------------------------------------------
	if (NetImgui::IsConnected())
		//-----------------------------------------------------------------------------------------
	{
		ImGui::TextUnformatted("Status: Connected");
		if (ImGui::Button("Disconnect"))//, ImVec2(120, 0)))
		{
			NetImgui::Disconnect();
		}
		return true;
	}
	//-----------------------------------------------------------------------------------------
	else if (NetImgui::IsConnectionPending())
		//-----------------------------------------------------------------------------------------
	{
		ImGui::TextUnformatted("Status: Waiting Server");
		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			NetImgui::Disconnect();
		}
		return true;
	}
	return false;
}

void ShowConnectToWindow(const char* zAppName, bool bCreateNewContext)
{
	if (ImGui::Begin("Connect to"))
	{
		if (!DisplayConnectedStatus())
		{
			ImGui::TextColored(ImVec4(0.1, 1, 0.1, 1), "Server Settings");
			ImGui::InputText("Hostname", sServerHostname, sizeof(sServerHostname));
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Address of PC running the netImgui server application. Can be an IP like 127.0.0.1");
			ImGui::InputInt("Port", &sServerPort);
			ImGui::NewLine();
			ImGui::Separator();
			if (ImGui::Button("Connect", ImVec2(ImGui::GetContentRegionAvailWidth(), 0)))
			{
				NetImgui::ConnectToApp(zAppName, sServerHostname, sServerPort, bCreateNewContext);
			}
		}
		ImGui::End();
	}

	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Attempt a connection to a remote netImgui server at the provided address.");
}

void ShowWaitForWindow(const char* zAppName, bool bCreateNewContext)
{
	if (ImGui::Begin("Wait for"))
	{
		if (!DisplayConnectedStatus())
		{
			ImGui::TextColored(ImVec4(0.1, 1, 0.1, 1), "Client Settings");
			ImGui::InputInt("Port", &sClientPort);
			ImGui::NewLine();
			ImGui::Separator();
			if (ImGui::Button("Listen", ImVec2(ImGui::GetContentRegionAvailWidth(), 0)))
			{
				NetImgui::ConnectFromApp(zAppName, sClientPort, bCreateNewContext);
			}
		}
		ImGui::End();
	}

	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Start listening for a connection request by a remote netImgui server, on the provided Port.");
}

void ClientUtil_ImGuiContent_Common(const char* zAppName, bool bCreateNewContext)
{
	(void)bCreateNewContext;
	if( ImGui::BeginMainMenuBar() )
	{
		if (ImGui::BeginMenu(zAppName))
		{
			ImGui::MenuItem("Connect to", NULL, &sbShowConnectToWindow);
			ImGui::MenuItem("Waiting for", NULL, &sbShowWaitForWindow);
			ImGui::MenuItem("Show ImGui Demo", NULL, &sbShowDemoWindow);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	if(sbShowConnectToWindow)
	{
		ShowConnectToWindow(zAppName, bCreateNewContext);
	}

	if (sbShowWaitForWindow)
	{
		ShowWaitForWindow(zAppName, bCreateNewContext);
	}

	if( sbShowDemoWindow )
	{
		ImGui::ShowDemoWindow(&sbShowDemoWindow);
	}
}

} // namespace SampleClient
