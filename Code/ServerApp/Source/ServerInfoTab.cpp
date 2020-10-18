#include "stdafx.h"
#include <stdio.h>
#include <algorithm>

#include <shellapi.h>	// To open webpage link
#include <NetImgui_Api.h>
#include "ServerInfoTab.h"
#include "ClientRemote.h"
#include "ClientConfig.h"
#include "ServerNetworking.h"

bool			gShowAbout				= false;
bool			gbShowServerConfig		= false;
constexpr char	kNetImguiURL[]			= "https://github.com/sammyfreg/netImgui";

// Client Config Layout
constexpr char	kColName_Name[]			= "Name";
constexpr char	kColName_HostName[]		= "HostName (IP)";
constexpr char	kColName_HostPort[]		= "Port";
constexpr char	kColName_AutoConnect[]	= "Auto";
constexpr float kColWidth_Port			= 50;
constexpr float kColWidth_AutoConnect	= 50;
constexpr float kColWidth_Edit			= 70;
constexpr float kColWidth_Connect		= 100;
constexpr float kColWidth_FixedConfig	= kColWidth_Port + kColWidth_AutoConnect + kColWidth_Edit + kColWidth_Connect;

// Client Connected Layout
constexpr char	kColName_Duration[]		= "Time";
constexpr char	kColName_ImguiVer[]		= "ImGui";
constexpr char	kColName_NetImguiVer[]	= "netImgui";
constexpr float kColWidth_ImGui			= 70;
constexpr float kColWidth_Duration		= kColWidth_AutoConnect + kColWidth_Edit;
constexpr float kColWidth_Disconnect	= kColWidth_Connect;
constexpr float kColWidth_FixedConnect	= kColWidth_Port + kColWidth_Duration + 2*kColWidth_ImGui + kColWidth_Disconnect;

//=================================================================================================
// Helper function that assign all columns width at once, from an initializer list
void SetupColumns(const char* columnName, bool useBorder, std::initializer_list<float> columnWidths)
//=================================================================================================
{
	ImGui::Columns(static_cast<int>(columnWidths.size()), columnName, useBorder);
	for(int i(0); i<static_cast<int>(columnWidths.size()); ++i)
		ImGui::SetColumnWidth(i, columnWidths.begin()[i]);
}

//=================================================================================================
// Edit the Server configuration
void ServerInfoTab_ServerConfig()
//=================================================================================================
{
	static int sEditPort = -1;
	if( gbShowServerConfig )
	{		
		if( sEditPort == -1 )
			sEditPort = static_cast<int>(ClientConfig::ServerPort);

		ImGui::OpenPopup("Server Configuration");
		if (ImGui::BeginPopupModal("Server Configuration", &gbShowServerConfig, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::NewLine();

			// --- Port ---						
			ImGui::TextUnformatted("Port waiting for connection requests");
			if (ImGui::InputInt("Port", &sEditPort, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue)) {
				sEditPort = std::min<int>(0xFFFF, std::max<int>(1, sEditPort));
			}
			ImGui::SameLine();
			if (ImGui::Button("Default")) {
				sEditPort = NetImgui::kDefaultServerPort;
			}

			// --- Save/Cancel ---
			ImGui::NewLine();
			ImGui::Separator();
			gbShowServerConfig &= !ImGui::Button("Cancel", ImVec2(ImGui::GetContentRegionAvailWidth() / 2.f, 0));
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Save", ImVec2(ImGui::GetContentRegionAvailWidth(), 0))) {
				ClientConfig::ServerPort = static_cast<uint32_t>(sEditPort);
				ClientConfig::SaveAll();
				gbShowServerConfig = false;
			}
			ImGui::EndPopup();
		}
	}
	else 
	{
		sEditPort = -1;
	}
}


//=================================================================================================
// Edit an existing or new Client Config entry
void ServerInfoTab_DrawClient_SectionConfig_PopupEdit(ClientConfig*& pEditClientCfg)
//=================================================================================================
{	
	bool bOpenEdit(pEditClientCfg);
	if (bOpenEdit)
	{		
		ImGui::OpenPopup("Edit Client Info");
		if (ImGui::BeginPopupModal("Edit Client Info", &bOpenEdit, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::NewLine();
			// --- Name ---
			ImGui::InputText("Display Name", pEditClientCfg->ClientName, sizeof(pEditClientCfg->ClientName));
			
			// --- IP Address ---
			ImGui::InputText("Host Name", pEditClientCfg->HostName, sizeof(pEditClientCfg->HostName));
			if( ImGui::IsItemHovered() ){
				ImGui::SetTooltip("IP address or name of the host to reach");
			}

			// --- Port ---
			int port = static_cast<int>(pEditClientCfg->HostPort);
			if( ImGui::InputInt("Host Port", &port, 1, 100, ImGuiInputTextFlags_EnterReturnsTrue) ){
				pEditClientCfg->HostPort = std::min<int>(0xFFFF, std::max<int>(1, port));
			}
			ImGui::SameLine();
			if (ImGui::Button("Default")){
				pEditClientCfg->HostPort = NetImgui::kDefaultClientPort;
			}

			// --- Auto ---
			ImGui::Checkbox("Auto Connect", &pEditClientCfg->ConnectAuto);
			
			// --- Save/Cancel ---
			ImGui::NewLine();
			ImGui::Separator();
			bOpenEdit &= !ImGui::Button("Cancel", ImVec2(ImGui::GetContentRegionAvailWidth()/2.f, 0));
			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Save", ImVec2(ImGui::GetContentRegionAvailWidth(), 0))){
				ClientConfig::SetConfig(*pEditClientCfg);
				ClientConfig::SaveAll();
				bOpenEdit = false;
			}
			ImGui::EndPopup();
		}
	}	

	if( !bOpenEdit ){
		SafeDelete(pEditClientCfg);
	}
}

//=================================================================================================
// Client Config Delete Confirmation
void ServerInfoTab_DrawClient_SectionConfig_PopupDelete(uint32_t& ClientId)
//=================================================================================================
{
	if( ClientId != ClientConfig::kInvalidRuntimeID )
	{
		ClientConfig config;
		bool bOpenDelConfirm( ClientConfig::GetConfigByID(ClientId, config) );
		if (bOpenDelConfirm)
		{
			ImGui::OpenPopup("Confirmation##DEL");
			if (ImGui::BeginPopupModal("Confirmation##DEL", &bOpenDelConfirm, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::NewLine();
				ImGui::TextUnformatted("Are you certain you want to remove\nthis client configuration ?");
				ImGui::SetNextItemWidth(-1.0f);
				if (ImGui::ListBoxHeader("##", 1, 2))
				{				
					ImGui::TextUnformatted(config.ClientName);
					ImGui::ListBoxFooter();
				}

				ImGui::NewLine();
				ImGui::Separator();
				bOpenDelConfirm &= !ImGui::Button("Cancel", ImVec2(ImGui::GetContentRegionAvailWidth()/2.f, 0));
				ImGui::SetItemDefaultFocus();

				ImGui::SameLine();
				if (ImGui::Button("Yes", ImVec2(ImGui::GetContentRegionAvailWidth(), 0))){
					ClientConfig::DelConfig(ClientId);
					ClientConfig::SaveAll();
					bOpenDelConfirm = false;
				}
				ImGui::EndPopup();
			}
		}

		if( !bOpenDelConfirm ){
			ClientId = ClientConfig::kInvalidRuntimeID;
		}
	}
}

//=================================================================================================
void ServerInfoTab_DrawClients_SectionConnected()
//=================================================================================================
{
	ImGui::NewLine();	

	int connectedCount(0);
	for (uint32_t i(1); i < ClientRemote::GetCountMax(); ++i)
		connectedCount += ClientRemote::Get(i).mbIsConnected ? 1 : 0;

	if( ImGui::CollapsingHeader("Connected", ImGuiTreeNodeFlags_DefaultOpen) )
	{		
		float widthAdjust = std::max<float>(50.f, ImGui::GetContentRegionAvailWidth() - kColWidth_FixedConnect);
		ImVec2 neededSize = ImVec2(0, ImGui::GetFrameHeightWithSpacing() * (std::max<int>(1,connectedCount) + 2));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 1.0f, 1.0f, 0.1f));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
		ImGui::BeginChildFrame(ImGui::GetID("ListConnected"), neededSize, 0);		
		ImGui::PopStyleVar();
		{
			//---------------------------------------------------------------------
			// List Header		
			ImGui::BeginChildFrame(ImGui::GetID("ListConnectHeader"), ImVec2(0, ImGui::GetFrameHeightWithSpacing()), ImGuiWindowFlags_NoScrollbar);
			{
				SetupColumns("ListConnectHeader", false, { widthAdjust*2.f/3.f, widthAdjust*1.f/3.f, kColWidth_ImGui, kColWidth_ImGui, kColWidth_Port, kColWidth_Duration, kColWidth_Disconnect });
				ImGui::TextUnformatted(kColName_Name); 			ImGui::NextColumn();
				ImGui::TextUnformatted(kColName_HostName);		ImGui::NextColumn();
				ImGui::TextUnformatted(kColName_ImguiVer);		ImGui::NextColumn();
				ImGui::TextUnformatted(kColName_NetImguiVer);	ImGui::NextColumn();
				ImGui::TextUnformatted(kColName_HostPort);		ImGui::NextColumn();				
				ImGui::TextUnformatted(kColName_Duration);		ImGui::NextColumn();
				ImGui::EndChildFrame();
			}

			//---------------------------------------------------------------------
			// List Content (skipping 1st element, because it's the ServerTab)
			SetupColumns("ListConnectContent", false, { widthAdjust*2.f/3.f, widthAdjust*1.f/3.f, kColWidth_ImGui, kColWidth_ImGui, kColWidth_Port, kColWidth_Duration, kColWidth_Disconnect });
			for (uint32_t i(1); i < ClientRemote::GetCountMax(); ++i)
			{
				ClientRemote& client = ClientRemote::Get(i);
				if( client.mbIsConnected )
				{
					ImGui::PushID(i);
					auto elapsedTime	= std::chrono::steady_clock::now() - client.mConnectedTime;
					int tmSec			= static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(elapsedTime).count() % 60);
					int tmMin			= static_cast<int>(std::chrono::duration_cast<std::chrono::minutes>(elapsedTime).count() % 60);
					int tmHour			= static_cast<int>(std::chrono::duration_cast<std::chrono::hours>(elapsedTime).count());					
					{
						if (client.mClientConfigID == ClientConfig::kInvalidRuntimeID) {
							ImGui::TextUnformatted("(No Config)"); ImGui::SameLine();
						}
						ImGui::TextUnformatted(client.mInfoName); 
					}ImGui::NextColumn();

					ImGui::TextUnformatted(client.mConnectHost);			ImGui::NextColumn();					
					ImGui::TextUnformatted(client.mInfoImguiVerName);		ImGui::NextColumn();
					ImGui::TextUnformatted(client.mInfoNetImguiVerName);	ImGui::NextColumn();
					ImGui::Text("%5i", client.mConnectPort);				ImGui::NextColumn();
					ImGui::Text("%03ih%02i:%02i", tmHour,tmMin,tmSec);		ImGui::NextColumn();
			
					if( !client.mbPendingDisconnect && ImGui::Button("Disconnect", ImVec2(80, 0)) ){
						client.mbPendingDisconnect = true;
					}ImGui::NextColumn();

					ImGui::PopID();
				}
			}
			if( connectedCount == 0 )
			{
				ImGui::Columns(1);
				ImGui::TextUnformatted("No netImgui Client connected for the moment.");
			}
		}
		ImGui::EndChildFrame();
		
		ImGui::TextUnformatted("Note: "); ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f,0.75f,0.75f,1.f));
		ImGui::PushTextWrapPos( ImGui::GetWindowContentRegionWidth() > 400 ? 0.f : 400.f ); 
		ImGui::TextUnformatted("Connections can either be initiated from this 'netImgui Server' (using 'Configurations' below) or a 'netImgui Client' can attempt reaching this server directly.");
		ImGui::PopTextWrapPos();
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar(1);
		ImGui::NewLine();
		ImGui::NewLine();
	}
}

//=================================================================================================
void ServerInfoTab_DrawClients_SectionConfig()
//=================================================================================================
{
	// Popups to edit or delete Client Configurations
	static uint32_t sPendingDeleteID		= ClientConfig::kInvalidRuntimeID;
	static ClientConfig* spPendingEditCfg	= nullptr;

	ServerInfoTab_DrawClient_SectionConfig_PopupEdit(spPendingEditCfg);
	ServerInfoTab_DrawClient_SectionConfig_PopupDelete(sPendingDeleteID);

	// Display list of Client Configuration	
	if( ImGui::CollapsingHeader("Configurations", ImGuiTreeNodeFlags_DefaultOpen) )
	{		
		ClientConfig clientConfig;
		float widthAdjust = std::max<float>(50.f, ImGui::GetContentRegionAvailWidth() - kColWidth_FixedConfig) / 2.f;
		ImVec2 neededSize = ImVec2(0, ImGui::GetFrameHeightWithSpacing() * (ClientConfig::GetConfigCount() + 2));
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.0f, 1.0f, 1.0f, 0.1f));		
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
		ImGui::BeginChildFrame(ImGui::GetID("ListConfig"), neededSize, 0);		
		ImGui::PopStyleVar();
		{
			//---------------------------------------------------------------------
			// List Header		
			ImGui::BeginChildFrame(ImGui::GetID("ListConfigHeader"), ImVec2(0, ImGui::GetFrameHeightWithSpacing()), ImGuiWindowFlags_NoScrollbar);			
			{				
				SetupColumns("ListConfigHeader", false, {widthAdjust, widthAdjust, kColWidth_Port, kColWidth_AutoConnect, kColWidth_Edit, kColWidth_Connect});
				ImGui::TextUnformatted(kColName_Name); 			ImGui::NextColumn();
				ImGui::TextUnformatted(kColName_HostName);		ImGui::NextColumn();
				ImGui::TextUnformatted(kColName_HostPort);		ImGui::NextColumn();
				ImGui::TextUnformatted(kColName_AutoConnect);	ImGui::NextColumn();				
				if (ImGui::Button("+") && !spPendingEditCfg) { spPendingEditCfg = new ClientConfig; } ImGui::NextColumn();				
				ImGui::EndChildFrame();
			}

			//---------------------------------------------------------------------
			// List Content			
			SetupColumns("ListConfigContent", false, {widthAdjust, widthAdjust, kColWidth_Port, kColWidth_AutoConnect, kColWidth_Edit, kColWidth_Connect});
			for (int i = 0; ClientConfig::GetConfigByIndex(i, clientConfig); ++i)
			{
				ImGui::PushID(i);
				
				// Config: Status and Name
				{
					ImGui::PushStyleColor(ImGuiCol_CheckMark, clientConfig.Connected ? ImVec4(0.7f, 1.f, 0.25f, 1.f) : ImVec4(1.f, 0.35f, 0.35f, 1.f));
					ImGui::RadioButton("##Connected", true);				
					ImGui::PopStyleColor();
					ImGui::SameLine(); 
					ImGui::TextUnformatted(clientConfig.ClientName); 
				}ImGui::NextColumn();

				// Config: Host info IP/Port
				ImGui::TextUnformatted(clientConfig.HostName);	ImGui::NextColumn();
				ImGui::Text("%5i", clientConfig.HostPort);		ImGui::NextColumn();

				// Config: AutoConnect changed				
				if( !clientConfig.Transient && ImGui::Checkbox("##auto", &clientConfig.ConnectAuto)) {
					ClientConfig::SetProperty_ConnectAuto(clientConfig.RuntimeID, clientConfig.ConnectAuto);
					ClientConfig::SaveAll();
				}ImGui::NextColumn();
				
				// Config: Remove client
				{
					if (ImGui::Button("-"))
						sPendingDeleteID = clientConfig.RuntimeID;
				
					if( !clientConfig.Transient ) {
						ImGui::SameLine(); 
						if ( ImGui::Button("..."))
						{
							spPendingEditCfg	= new ClientConfig;
							*spPendingEditCfg	= clientConfig;
						}
					}
				}ImGui::NextColumn();

				// Config: Connection attempt
				{
					if( clientConfig.Transient ){
						ImGui::TextUnformatted("(Request)");
					}
					else if (!clientConfig.Connected && !clientConfig.ConnectRequest && ImGui::Button("Connect", ImVec2(80,0 )) ){
						ClientConfig::SetProperty_ConnectRequest(clientConfig.RuntimeID, true);
					}
				}ImGui::NextColumn();

				ImGui::PopID();
			}
			ImGui::Columns(1);
		}
		ImGui::EndChildFrame();
		ImGui::PopStyleColor(1);
		ImGui::PopStyleVar(1);
	}
}

//=================================================================================================
// Window displaying active connexion and Client list configured to attempt connexion with
void ServerInfoTab_DrawClients()
//=================================================================================================
{
	if( ImGui::Begin("Clients") )
	{   
		ServerInfoTab_DrawClients_SectionConnected();
		ServerInfoTab_DrawClients_SectionConfig();
		ImGui::NewLine();
	}
	ImGui::End();
}

//=================================================================================================
void ServerInfoTab_AboutDialog()
//=================================================================================================
{
	static bool sbURLHover = false;
	if( gShowAbout )
	{
		if( ImGui::Begin("About netImgui", &gShowAbout, ImGuiWindowFlags_AlwaysAutoResize) )
		{
			ImGui::NewLine();
			ImGui::Text("Version : %s", NETIMGUI_VERSION);
			ImGui::NewLine();
			ImGui::Text("For more informations : ");
			
			ImGui::TextColored(sbURLHover ? ImColor(0.8f, 0.8f, 1.f,1.f) : ImColor(0.5f, 0.5f, 1.f,1.f), kNetImguiURL);
			sbURLHover = ImGui::IsItemHovered();
			if( sbURLHover && ImGui::IsMouseClicked(ImGuiMouseButton_Left) )
				ShellExecuteA(0, 0, kNetImguiURL, 0, 0 , SW_SHOW );

			ImGui::NewLine();
			ImGui::TextUnformatted("Note: Commandline can be used to connect to a Client."); 
			ImGui::TextColored(ImColor(0.6f,0.65f,0.6f,1.f), "Syntax : (HostName);(HostPort)");	
			ImGui::TextColored(ImColor(0.6f,0.65f,0.6f,1.f), "Example: netImgui_Server.exe 127.0.0.1;8889");
			ImGui::NewLine();
			ImGui::Separator();
			if( ImGui::Button("Close", ImVec2(ImGui::GetContentRegionAvailWidth(), 0)) ) 
				gShowAbout = false;
		}
		ImGui::End();
	}
	else
		sbURLHover = false;
}
//=================================================================================================
// Draw
//=================================================================================================
void ServerInfoTab_Draw()
{	
	if (NetImgui::NewFrame(true))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3,3+6) );		
		if( ImGui::BeginMainMenuBar() )
		{
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.8, 0.8, 0.8, 0.9));
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(3,3) );
			ImGui::SetCursorPosY(6);
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, gShowAbout ? 1.f : 0.f);
			gShowAbout			^= ImGui::Button(" About ");
			
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, gbShowServerConfig ? 1.f : 0.f);
			gbShowServerConfig	^= ImGui::Button("  ...  ");

			ImGui::SetCursorPosY(0);
			if( NetworkServer::IsWaitingForConnection() )
				ImGui::Text("Waiting Connections on port: %i", static_cast<int>(ClientConfig::ServerPort));
			else
				ImGui::Text("Unable to wait Connections on port: %i", static_cast<int>(ClientConfig::ServerPort));
			ImGui::PopStyleColor();
			ImGui::PopStyleVar(3);
			ImGui::EndMainMenuBar();			
		}
		ImGui::PopStyleVar(1);

		ServerInfoTab_ServerConfig();
		ServerInfoTab_DrawClients();
		ServerInfoTab_AboutDialog();
		NetImgui::EndFrame();
	}
}

//=================================================================================================
// Startup
//=================================================================================================
bool ServerInfoTab_Startup()
{	
	if( !NetImgui::Startup() )
		return false;

	ImGui::SetCurrentContext(ImGui::CreateContext());
	ImGuiIO& io = ImGui::GetIO();	

	// Build Font
	io.Fonts->AddFontDefault();
	io.Fonts->Build();
	io.Fonts->TexID = reinterpret_cast<ImTextureID*>(0); // Store our identifier

	if( NetImgui::ConnectToApp("netImgui", "localhost", ClientConfig::ServerPort, false) )
	{
		// Wait for 'ServerInfoTab' client to connect first in slot 0
		while( !ClientRemote::Get(0).mbIsConnected )
			std::this_thread::yield();
		return true;
	}
	return false;
}

//=================================================================================================
// Shutdown
//=================================================================================================
void ServerInfoTab_Shutdown()
{	
	ImGui::DestroyContext(ImGui::GetCurrentContext());
}
