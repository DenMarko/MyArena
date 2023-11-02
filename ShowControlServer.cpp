#include "ShowControlServer.h"
#include "CNotification.h"

#include "resource.h"
#include <algorithm>
#include "stb_image.h"

class CLoadDataMap : public ITimerEvent
{
public:
	virtual TimerResult OnTimer(void *pData) override
	{
		reinterpret_cast<CControlServer*>(pData)->SetMaps();
		return TimerResult::Time_Stop;
	}
	virtual void OnTimerEnd(void *pData) override
	{}
};

class CLoadDataStatus : public ITimerEvent
{
public:
	virtual TimerResult OnTimer(void *pData) override
	{
		reinterpret_cast<CControlServer*>(pData)->SetStatus();
		return TimerResult::Time_Stop;
	}
	virtual void OnTimerEnd(void *pData) override
	{}
};

CControlServer::CControlServer(ID3D11Device* pDevice) :  g_pDevice(pDevice),
		p_gMaps(nullptr), p_gStatus(nullptr), Status(nullptr),
		im_texture(nullptr), im_texture_url(nullptr), IsStatusReady(true)
{
	timer->CreateTimer(make_shared<CLoadDataStatus>(), 0.1f, this);
	timer->CreateTimer(make_shared<CLoadDataMap>(), 0.1f, this);

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
			char *pData = reinterpret_cast<char*>(LockResource(hGlob));

			Utilite::CArray<char> bite;
			bite.push("\x42\x4D\x36\xE1\x00\x00\x00\x00\x00\x00\x36\x00\x00\x00", 14); bite.push(pData, size_);
			LoadBuffer(bite.data(), bite.Size(), &im_texture, &im_width, &im_height);

			FreeResource(hGlob);
			bite.clear();
		}
	}
}

void CControlServer::SetStatus()
{
	IsStatusReady = false;

	Status = reinterpret_cast<GetStatus *>(pUrls->GetData(STATUS));

	int iX = 0;
	int iY = 0;
	int iComp = 0;

	if (Status != nullptr)
	{
		prevMap = Status->data->mapname;

		Utilite::CArray<char> gImgMap;
		pUrls->LoadImageMap(Status->MapImg, &gImgMap); gImgMap.push('\0');
		if (stbi_info_from_memory(reinterpret_cast<const unsigned char*>(gImgMap.data()), gImgMap.Size(), &iX, &iY, &iComp))
		{
			if (!LoadBuffer(gImgMap.data(), gImgMap.Size(), &im_texture_url, &im_width_url, &im_height_url))
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
}

void CControlServer::SetMaps()
{
	if (g_pGlob->IsMapsReloads)
	{
		g_pGlob->IsMapsReloads = false;
		RELEASE(p_gMaps)
	}

	if (p_gMaps == nullptr)
	{
		p_gMaps = reinterpret_cast<GetMapList*>(pUrls->GetData(GET_MAPS));
		if(p_gMaps != nullptr)
			std::sort(p_gMaps->maps.begin(), p_gMaps->maps.end());
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
					RELEASE(p_gStatus)

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

		switch (p_gStatus->online)
		{
		case 0:
			ImGui::Text(gLangManager->GetLang("Status: Disabled"));
			break;
		case 1:
			ImGui::Text(gLangManager->GetLang("Status: Enabled"));
			break;
		case 2:
			ImGui::Text(gLangManager->GetLang("Status: Reboot"));
			break;
		default:
			ImGui::Text(u8" ");
			break;
		}

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
			timer->AddNextFrame([online = p_gStatus->online]()
			{
				if (online == 0)
				{
					auto res = reinterpret_cast<CmdResult *>(pUrls->GetData(START));

					if (res->status == OK)
					{
						g_pNotif->Notificatio(res->msg.c_str());
					}

					delete res;
				}
				else
				{
					g_pNotif->Notificatio(u8"%s", online == 1 ? gLangManager->GetLang("The server is already enabled") : online == 2 ? gLangManager->GetLang("The server reboots") : u8"NULL");
				}
			});
		}
		ImGui::SameLine();
		if (ImGui::Button(gLangManager->GetLang("Stop"), ImVec2(80, 0)))
		{
			timer->AddNextFrame([online = p_gStatus->online]()
			{
				if (online == 1)
				{
					auto res = reinterpret_cast<CmdResult *>(pUrls->GetData(STOP));

					if (res->status == OK)
					{
						g_pNotif->Notificatio(res->msg.c_str());
					}

					delete res;
				}
				else
				{
					g_pNotif->Notificatio(u8"%s", online == 0 ? gLangManager->GetLang("The server is already down") : online == 2 ? gLangManager->GetLang("The server reboots") : u8"NULL");
				}
			});
		}
		ImGui::SameLine();
		if (ImGui::Button(gLangManager->GetLang("Restart"), ImVec2(80, 0)))
		{
			timer->AddNextFrame([online = p_gStatus->online]()
			{
				if (online == 1)
				{
					auto res = reinterpret_cast<CmdResult *>(pUrls->GetData(RESTART));
					if (res->status == OK)
					{
						g_pNotif->Notificatio(res->msg.c_str());
					}

					delete res;
				}
				else
				{
					g_pNotif->Notificatio(u8"%s", online == 0 ? gLangManager->GetLang("The server is down") : online == 2 ? gLangManager->GetLang("The server is already rebooting") : u8"NULL");
				}
			});
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
		if (p_gMaps != nullptr && p_gMaps->status == OK)
		{
			if (ImGui::BeginCombo(gLangManager->GetLang("Change map"), p_gStatus->data->mapname.c_str(), ImGuiComboFlags_NoPreview))
			{
				for (auto& str : p_gMaps->maps)
				{
					const bool is_select = (str == p_gStatus->data->mapname);
					if(ImGui::Selectable(str.c_str(), is_select))
					{
						auto res = reinterpret_cast<CmdResult *>(pUrls->GetData(CHANGE_LEVEL, str.c_str()));
						if (res->status == OK)
						{
							g_pNotif->Notificatio(u8"%s", res->msg.c_str());
						}
						delete res;
					}

					if(is_select)
						ImGui::SetItemDefaultFocus();

				}

				ImGui::EndCombo();
			}
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

	Status = reinterpret_cast<GetStatus *>(pUrls->GetData(STATUS));
	if (Status == nullptr)
	{
		IsStatusReady = true;
		return Time_Continue;
	}
	else if (Status->status == ERRORS)
	{
		delete Status;
		Status = nullptr;
		IsStatusReady = true;
		return Time_Continue;
	}

	SetMaps();

	if (prevMap != Status->data->mapname)
	{
		prevMap = Status->data->mapname;

		int iX = 0;
		int iY = 0;
		int iComp = 0;
		Utilite::CArray<char> gImgMap;
		pUrls->LoadImageMap(Status->MapImg, &gImgMap); gImgMap.push('\0');
		if (stbi_info_from_memory(reinterpret_cast<const unsigned char*>(gImgMap.data()), gImgMap.Size(), &iX, &iY, &iComp))
		{
			if (im_texture_url)
			{
				im_texture_url->Release();
				im_texture_url = nullptr;
			}

			if (!LoadBuffer(gImgMap.data(), gImgMap.Size(), &im_texture_url, &im_width_url, &im_height_url))
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