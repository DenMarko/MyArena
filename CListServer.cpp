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

		ImGui::Spacing();
		ImGui::SeparatorText(gLangManager->GetLang("List of servers"));
		ImGui::Spacing();

		ImVec2 wSize = ImGui::GetWindowSize();
		if (ImGui::BeginTable("List of servers", 2, ImGuiTableFlags_Borders |
													ImGuiTableFlags_NoBordersInBodyUntilResize |
													ImGuiTableFlags_SizingStretchProp, ImVec2(0.f, (wSize.y - 98.f))))
		{
			ImGui::TableSetupColumn(gLangManager->GetLang("Server name"), ImGuiTableColumnFlags_WidthFixed, 200.f);
			ImGui::TableSetupColumn(gLangManager->GetLang("Active"));
			ImGui::TableHeadersRow();
			size_t i = 0;
			auto iter = g_pGlob->token.begin();
			for ( ; iter != g_pGlob->token.end(); )
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				char label[32];
				sprintf_s(label, u8"%d: %s", (i + 1), (*iter)->NServer.c_str());
				if(ImGui::Selectable(label, (*iter)->IsSelect, ImGuiSelectableFlags_SpanAllColumns)) {	(*iter)->IsSelect = true;	}

				if (ImGui::BeginPopupContextItem())
				{
					(*iter)->IsSelect = true;

					if (!(*iter)->IsActive)
					{
						if (ImGui::Button(gLangManager->GetLang("Activate")))
						{
							(*iter)->IsActive = true;
							g_pGlob->IsWriteAtiveToken = true;
							g_pGlob->IsWriteToken = true;

							ImGui::CloseCurrentPopup();
							(*iter)->IsSelect = false;
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
								if ((*iter)->IsActive == false)
								{
									(*iter)->IsActive = true;
									g_pGlob->IsWriteAtiveToken = true;
								}
							}
							else
							{
								iter--;
								if ((*iter)->IsActive == false)
								{
									(*iter)->IsActive = true;
									g_pGlob->IsWriteAtiveToken = true;
								}
							}
						}
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}
				if (!ImGui::IsPopupOpen(label)) {	(*iter)->IsSelect = false;	}
				ImGui::TableNextColumn();
				ImGui::Text(u8"%s", (*iter)->IsActive ? gLangManager->GetLang("Active") : gLangManager->GetLang("Not active"));
				
				iter++;
				i++;
			}

			ImGui::EndTable();
		}

		ImGui::SetCursorPos(ImVec2((wSize.x * 0.5f) * 0.5f, (wSize.y - 30.f)));
		if (ImGui::Button(gLangManager->GetLang("Add server"), ImVec2(ImGui::GetWindowSize().x * 0.5f, 0.f)))
		{
			g_pGlob->IsTocken = false;
		}

		ImGui::End();
	}
}
