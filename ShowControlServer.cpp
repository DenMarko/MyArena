#include "ShowControlServer.h"
#include "CNotification.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "resource.h"

CControlServer::CControlServer(ID3D11Device* pDevice) : 
g_pDevice(pDevice),
p_gMaps(nullptr),
p_gStatus(nullptr),
Status(nullptr),
im_texture(nullptr),
im_texture_url(nullptr),
IsStatusReady(true)
{
	p_gStatus = reinterpret_cast<GetStatus *>(pUrls->GetData(COMAND::COMMAND_STATUS));
	p_gMaps = reinterpret_cast<GetMapList *>(pUrls->GetData(COMAND::COMMAND_GET_MAPS));

	int iX = 0;
	int iY = 0;
	int iComp = 0;

	if (p_gStatus != nullptr)
	{
		prevMap.append(p_gStatus->data->mapname);

		std::vector<char> gImgMap;
		pUrls->LoadImageMap(p_gStatus->MapImg, &gImgMap);
		if (stbi_info_from_memory(reinterpret_cast<const unsigned char*>(gImgMap.data()), gImgMap.size(), &iX, &iY, &iComp))
		{
			IMGUI_DEBUG_LOG("Image info: X[%d] Y[%d] comp[%d]\n", iX, iY, iComp);
			if (!LoadBuffer(gImgMap.data(), gImgMap.size(), &im_texture_url, &im_width_url, &im_height_url))
			{
				if (im_texture_url)
				{
					im_texture_url->Release();
					im_texture_url = nullptr;
				}
			}
		} else { IMGUI_DEBUG_LOG("Image info is error\n"); }
	}

	if (p_gStatus == nullptr)
	{
		prevMap = "none";
		p_gStatus = new GetStatus();
	}

	HMODULE hM = GetModuleHandle(nullptr);
	HRSRC hRes = FindResource(hM, MAKEINTRESOURCE(IDB_BITMAP2), RT_BITMAP);
	if (hRes)
	{
		HGLOBAL hGlob = LoadResource(hM, hRes);
		if (hGlob)
		{
			DWORD size_ = SizeofResource(hM, hRes);
			BYTE *pData = reinterpret_cast<BYTE*>(LockResource(hGlob));

			const char* b = "\x42\x4D\x36\xE1\x00\x00\x00\x00\x00\x00\x36\x00\x00\x00";
			vector<BYTE> bite(14);
			memcpy(bite.data(), b, 14);

			bite.resize(size_+14);
			memcpy(bite.data()+14, pData, size_);

			LoadBuffer((char *)bite.data(), bite.size(), &im_texture, &im_width, &im_height);

			FreeResource(hGlob);
		}
	}
}

bool CControlServer::LoadBuffer(char* filebuf, const size_t fSize, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height)
{
	int image_width = 0;
	int image_height = 0;
	unsigned char* image_data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(filebuf), fSize, &image_width, &image_height, NULL, 4);
	if (image_data == NULL)
		return false;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = image_width;
	desc.Height = image_height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;

	ID3D11Texture2D *pTexture = NULL;
	D3D11_SUBRESOURCE_DATA subResource;
	subResource.pSysMem = image_data;
	subResource.SysMemPitch = desc.Width * 4;
	subResource.SysMemSlicePitch = 0;
	g_pDevice->CreateTexture2D(&desc, &subResource, &pTexture);

	// Create texture view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	g_pDevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
	pTexture->Release();

	*out_width = image_width;
	*out_height = image_height;
	stbi_image_free(image_data);

	return true;
}

void CControlServer::OnUIRender()
{
	if (g_pGlob->IsShowControlServer)
	{
		if(IsStatusReady)
		{
			if (Status)
			{
				if (p_gStatus != Status)
				{
					delete p_gStatus;
					p_gStatus = Status;
					Status = nullptr;
				}
			}
		}

		ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin(gLangManager->GetLang("Server management"), &g_pGlob->IsShowControlServer))
		{
			ImGui::End();
			return;
		}

		ImGui::SeparatorText(gLangManager->GetLang("Server information"));
		ImGui::Spacing();

		ImGui::BeginChild("Left", ImVec2(160, 140));
		if (im_texture_url == nullptr)
		{
			if (im_texture != nullptr)
			{
				ImGui::Image((void *)im_texture, ImVec2(static_cast<float>(im_width), static_cast<float>(im_height)));
			}
		}
		else
		{
			ImGui::Image((void *)im_texture_url, ImVec2(static_cast<float>(im_width_url), static_cast<float>(im_height_url)));
		}
		ImGui::EndChild();
		ImGui::SameLine();

		ImGui::BeginGroup();
		ImGui::BeginChild("Right", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing()-160)));

		ImGui::Text(u8"%s", p_gStatus->data->hostname.c_str());

		if(p_gStatus->online == 0)
			ImGui::Text(gLangManager->GetLang("Status: Disabled"));
		else if(p_gStatus->online == 1)
			ImGui::Text(gLangManager->GetLang("Status: Enabled"));
		else if(p_gStatus->online == 2)
			ImGui::Text(gLangManager->GetLang("Status: Reboot"));
		else
			ImGui::Text(u8" ");

		ImGui::Text(u8"IP: %s", p_gStatus->data->addres.c_str());
		ImGui::Text(u8"%s: %s", gLangManager->GetLang("Map"), p_gStatus->data->mapname.c_str());
		ImGui::Text(u8"%s: %i/%i", gLangManager->GetLang("Players"), p_gStatus->data->numPlayers, p_gStatus->data->maxPlayers);
		ImGui::Text(u8"%s: %d", gLangManager->GetLang("Server ID"), p_gStatus->server_id);
		ImGui::Text(u8"%s: %d", gLangManager->GetLang("Days until the end of the lease"), p_gStatus->server_daytoblock);

		ImGui::EndChild();
		ImGui::EndGroup();

		ImGui::Spacing();
		ImGui::SeparatorText(gLangManager->GetLang("Management"));
		ImGui::Spacing();

		if (ImGui::Button(gLangManager->GetLang("Start"), ImVec2(80, 0)))
		{
			if (p_gStatus->online == 0)
			{
				auto res = reinterpret_cast<CmdResult *>(pUrls->GetData(COMAND::COMMAND_START));

				if (res->status == STATUS::STATUS_OK)
				{
					g_pNotif->Notificatio(res->msg.c_str());
				}

				delete res;
			}
			else
			{
				g_pNotif->Notificatio(u8"%s", p_gStatus->online == 1 ? gLangManager->GetLang("The server is already enabled") : p_gStatus->online == 2 ? gLangManager->GetLang("The server reboots") : u8"NULL");
			}
		}
		ImGui::SameLine();
		if (ImGui::Button(gLangManager->GetLang("Stop"), ImVec2(80, 0)))
		{
			if (p_gStatus->online == 1)
			{
				auto res = reinterpret_cast<CmdResult *>(pUrls->GetData(COMAND::COMMAND_STOP));

				if (res->status == STATUS::STATUS_OK)
				{
					g_pNotif->Notificatio(res->msg.c_str());
				}

				delete res;
			}
			else
			{
				g_pNotif->Notificatio(u8"%s", p_gStatus->online == 0 ? gLangManager->GetLang("The server is already down") : p_gStatus->online == 2 ? gLangManager->GetLang("The server reboots") : u8"NULL");
			}
		}
		ImGui::SameLine();
		if (ImGui::Button(gLangManager->GetLang("Restart"), ImVec2(80, 0)))
		{
			if (p_gStatus->online == 1)
			{
				auto res = reinterpret_cast<CmdResult *>(pUrls->GetData(COMAND::COMMAND_RESTART));

				if (res->status == STATUS::STATUS_OK)
				{
					g_pNotif->Notificatio(res->msg.c_str());
				}

				delete res;
			}
			else
			{
				g_pNotif->Notificatio(u8"%s", p_gStatus->online == 0 ? gLangManager->GetLang("The server is down") : p_gStatus->online == 2 ? gLangManager->GetLang("The server is already rebooting") : u8"NULL");
			}
		}
		ImGui::SameLine();
		if (ImGui::Button(gLangManager->GetLang("Go to the server"), ImVec2(130, 0)))
		{
			if (p_gStatus->online == 1)
			{
				SHELLEXECUTEINFO info = {0};
				info.cbSize = sizeof(SHELLEXECUTEINFO);
				info.lpVerb = "open";
				info.lpFile = p_gStatus->JoinServer.c_str();
				info.nShow = SW_SHOWNORMAL;

				ShellExecuteEx(&info);
			}
			else
			{
				g_pNotif->Notificatio(u8"%s", p_gStatus->online == 0 ? gLangManager->GetLang("The server is down") : p_gStatus->online == 2 ? gLangManager->GetLang("The server reboots") : u8"NULL");
			}
		}
		ImGui::SameLine();

		static int item_cur_id = 0;
		const char *comb_prev_val = p_gMaps != nullptr ? p_gMaps->status == STATUS::STATUS_OK ? p_gMaps->maps[item_cur_id].c_str() : '\0' : '\0';
		if (ImGui::BeginCombo(gLangManager->GetLang("Change map"), comb_prev_val, ImGuiComboFlags_NoPreview))
		{
			int n = 0;
			for (auto& str : p_gMaps->maps)
			{
				const bool is_select = (item_cur_id == n);
				if(ImGui::Selectable(str.c_str(), is_select))
				{
					item_cur_id = n;
					auto res = reinterpret_cast<CmdResult *>(pUrls->GetData(COMAND::COMMAND_CHANGE_LEVEL, str.c_str()));
					if (res->status == STATUS::STATUS_OK)
					{
						g_pNotif->Notificatio(u8"%s", res->msg.c_str());
					}
					delete res;
				}

				if(is_select)
					ImGui::SetItemDefaultFocus();

				n++;
			}

			ImGui::EndCombo();
		}

		ImGui::Spacing();
		ImGui::SeparatorText(gLangManager->GetLang("Players"));
		ImGui::Spacing();

		static ImGuiTableFlags mFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBodyUntilResize | ImGuiTableFlags_SizingStretchProp;
		if (ImGui::BeginTable(gLangManager->GetLang("Players"), 3, mFlags, ImVec2(0, -ImGui::GetFrameHeightWithSpacing())))
		{
			ImGui::TableSetupColumn(gLangManager->GetLang("Player name"), ImGuiTableColumnFlags_WidthFixed, 300.f);
			ImGui::TableSetupColumn(gLangManager->GetLang("Frags"), ImGuiTableColumnFlags_WidthFixed, 50.f);
			ImGui::TableSetupColumn(gLangManager->GetLang("Time in game"), ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableHeadersRow();

			for (auto &p : p_gStatus->data->players)
			{
				ImGui::TableNextRow();
				for(int col = 0; col < 3; col++)
				{
					ImGui::TableSetColumnIndex(col);
					if (col == 0)
					{
						ImGui::Text(u8"%s", p.name.c_str());
					}
					else if (col == 1)
					{
						ImGui::Text(u8"%i", p.score);
					}
					else if (col == 2)
					{
						ImGui::Text(u8"%s", p.time_to_game.c_str());
					}
				}
			}

			ImGui::EndTable();
		}

		ImGui::End();
	}
}

TimerResult CControlServer::OnTimer(void *pData)
{
	IsStatusReady = false;

	Status = reinterpret_cast<GetStatus *>(pUrls->GetData(COMAND::COMMAND_STATUS));
	if (Status == nullptr)
	{
		IsStatusReady = true;
		return Time_Continue;
	}
	else if (Status->status == STATUS::STATUS_ERROR)
	{
		delete Status;
		Status = nullptr;
		IsStatusReady = true;
		return Time_Continue;
	}

	if (p_gMaps == nullptr) { p_gMaps = reinterpret_cast<GetMapList *>(pUrls->GetData(COMAND::COMMAND_GET_MAPS)); }
	if (prevMap != Status->data->mapname)
	{
		prevMap = Status->data->mapname;
		IMGUI_DEBUG_LOG("%s\n", prevMap.c_str());

		int iX = 0;
		int iY = 0;
		int iComp = 0;
		std::vector<char> gImgMap;
		pUrls->LoadImageMap(Status->MapImg, &gImgMap);
		if (stbi_info_from_memory(reinterpret_cast<const unsigned char*>(gImgMap.data()), gImgMap.size(), &iX, &iY, &iComp))
		{
			IMGUI_DEBUG_LOG("Image info: X[%d] Y[%d] comp[%d]\n", iX, iY, iComp);

			if (im_texture_url)
			{
				im_texture_url->Release();
				im_texture_url = nullptr;
			}

			if (!LoadBuffer(gImgMap.data(), gImgMap.size(), &im_texture_url, &im_width_url, &im_height_url))
			{
				if (im_texture_url)
				{
					im_texture_url->Release();
					im_texture_url = nullptr;
				}
			}
		}
	}
	IsStatusReady = true;

	return TimerResult::Time_Continue;
}