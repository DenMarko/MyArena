#include "CListServer.h"
#include "imgui.h"
#include "imgui_internal.h"

CListServer::CListServer()
{
	for (auto &tok : g_pGlob->token)
	{
		TokenResult * stats = reinterpret_cast<TokenResult *>(pUrls->StatusToken(tok->token.c_str()));
		if (stats != nullptr)
		{
			if(stats->status == STATUS::STATUS_OK)
				tok->NServer = stats->SServer->data->hostname;
		
			delete stats;
		}
	}
}

CListServer::~CListServer()
{
}

void CListServer::OnUIRender()
{
	if (g_pGlob->IsShowListServer)
	{
		ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin(gLangManager->GetLang("List of servers"), &g_pGlob->IsShowListServer))
		{
			ImGui::End();
			return;
		}

		ImGui::SetCursorPos(ImVec2((ImGui::GetWindowSize().x * 0.5f) * 0.5f, ImGui::GetCursorPos().y));
		if (ImGui::Button(gLangManager->GetLang("Add server"), ImVec2(ImGui::GetWindowSize().x * 0.5f, 0.f)))
		{
			g_pGlob->IsTocken = false;
		}

		ImGui::Spacing();
		ImGui::SeparatorText(gLangManager->GetLang("List of servers"));
		ImGui::Spacing();

		static ImGuiTableFlags tFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_NoBordersInBodyUntilResize | ImGuiTableFlags_SizingStretchProp;
		if (ImGui::BeginTable("List of servers", 2, tFlags, ImVec2(0.f, 0.f)))
		{
			ImGui::TableSetupColumn(gLangManager->GetLang("Server name"), ImGuiTableColumnFlags_WidthFixed, 200.f);
			ImGui::TableSetupColumn(gLangManager->GetLang("Active"));
			ImGui::TableHeadersRow();

			for (size_t i = 0; i < g_pGlob->token.size(); i++)
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				char label[32];
				sprintf_s(label, u8"%d: %s", (i + 1), g_pGlob->token[i]->NServer.c_str());
				if(ImGui::Selectable(label, &g_pGlob->token[i]->IsSelect, ImGuiSelectableFlags_SpanAllColumns))
				{
					ImGui::OpenPopup("open_menu_select");
				}
				ImGui::TableNextColumn();
				ImGui::Text(u8"%s", g_pGlob->token[i]->IsActive ? gLangManager->GetLang("Active") : gLangManager->GetLang("Not active"));
			}

			ImGui::EndTable();
		}

		if (ImGui::BeginPopupContextItem("open_menu_select", ImGuiPopupFlags_MouseButtonLeft))
		{
			for (auto iter = g_pGlob->token.begin(); iter != g_pGlob->token.end(); )
			{
				if((*iter)->IsSelect)
				{
					if (!(*iter)->IsActive)
					{
						if (ImGui::Button(gLangManager->GetLang("Activate")))
						{
							(*iter)->IsActive = true;
							g_pGlob->IsWriteAtiveToken = true;
							g_pGlob->IsWriteToken = true;

							ImGui::CloseCurrentPopup();
							(*iter)->IsSelect = false;
							break;
						}
					}
					
					if (ImGui::Button(gLangManager->GetLang("Remove")))
					{
						iter = g_pGlob->token.erase(iter);
						g_pGlob->IsWriteToken = true;

						if (g_pGlob->token.size() == 0)
						{
							g_pGlob->IsTocken = false;
						}
						else
						{
							if (iter != g_pGlob->token.end())
							{
								(*iter)->IsActive = true;
								g_pGlob->IsWriteAtiveToken = true;
							}
							else
							{
								iter--;
								(*iter)->IsActive = true;
								g_pGlob->IsWriteAtiveToken = true;
							}
						}
						ImGui::CloseCurrentPopup();

						break;
					}
				}
				iter++;
			}
			ImGui::EndPopup();
		}

		if (!ImGui::IsPopupOpen("open_menu_select"))
		{
			for (auto &tok : g_pGlob->token)
			{
				if (tok->IsSelect)
				{
					tok->IsSelect = false;
					break;
				}
			}
		}

		ImGui::End();
	}
}
