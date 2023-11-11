#include "CListServer.h"
#include "imgui.h"
#include "imgui_internal.h"

class CLoadDataListServer : public ITimerEvent
{
public:
	virtual TimerResult OnTimer(void *pData) override
	{
		if (reinterpret_cast<CListServer*>(pData)->LoadData()) {
			return TimerResult::Time_Stop;
		}
		
		return TimerResult::Time_Continue;
	}
	virtual void OnTimerEnd(void *pData) override
	{}
};

CListServer::CListServer() : pTimerLoadData(nullptr)
{
	pTimerLoadData = timer->CreateTimer(make_shared<CLoadDataListServer>(), 0.1f, this);
}

CListServer::~CListServer()
{
	
}

bool CListServer::LoadData()
{
	auto myLocalTimes = [](STokenList::_times_ *res, const time_t& time) {
		long seconds = static_cast<long>(time);
		res->second = seconds % 60;
		seconds /= 60;
		res->minute = seconds % 60;
		seconds /= 60;
		res->hour = seconds % 24;
		seconds /= 24;

		const int daysInYear = 365;
		const int daysInLeapYear = 366;

		int year = 1970;
		int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

		while (true) {
			int daysInCurrentYear = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)) ? daysInLeapYear : daysInYear;
			if (seconds < daysInCurrentYear) {
				break;
			}
			year++;
			seconds -= daysInCurrentYear;
		}

		res->year = year;

		for (int month = 1; month <= 12; ++month) {
			int daysInCurrentMonth = daysInMonth[month];
			if (month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0))) {
				daysInCurrentMonth = 29;
			}

			if (seconds < daysInCurrentMonth) {
				res->month = month;
				res->day = seconds + 1;
				break;
			}

			seconds -= daysInCurrentMonth;
		}
		return res;
	};

	for (auto &tok : g_pGlob->token)
	{
		TokenResult *stats = reinterpret_cast<TokenResult *>(pUrls->StatusToken(tok->token.c_str()));
		if (stats != nullptr)
		{
			if (stats->status == OK)
			{
				tok->NServer = stats->SServer->data->hostname;
				myLocalTimes(&tok->data_ty_block, static_cast<time_t>(stats->SServer->server_dateblock));
			}
		
			delete stats;
		} else {
			pTimerLoadData->SetNewInterval(5.0f);
			return false;
		}
	}
	return true;
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
		if (ImGui::BeginTable("List of servers", 3, ImGuiTableFlags_Borders |
													ImGuiTableFlags_NoBordersInBodyUntilResize |
													ImGuiTableFlags_SizingStretchProp, ImVec2(0.f, (wSize.y - 98.f))))
		{
			ImGui::TableSetupColumn(gLangManager->GetLang("Server name"), ImGuiTableColumnFlags_WidthFixed, 200.f);
			ImGui::TableSetupColumn(gLangManager->GetLang("Active"), ImGuiTableColumnFlags_WidthFixed, 100.f);
			ImGui::TableSetupColumn(gLangManager->GetLang("Paid until"));
			ImGui::TableHeadersRow();
			size_t i = 0;
			auto iter = g_pGlob->token.begin();
			while (iter != g_pGlob->token.end())
			{
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				char label[32];
				sprintf_s(label, u8"%d: %s", (i + 1), (*iter)->NServer.c_str());
				if (ImGui::Selectable(label, (*iter)->IsSelect, ImGuiSelectableFlags_SpanAllColumns)) {	(*iter)->IsSelect = true;	}
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
						ImGui::EndPopup();
						continue;
					}
					ImGui::EndPopup();
				}
				if (!ImGui::IsPopupOpen(label))
				{
					if((*iter)->IsSelect)
						(*iter)->IsSelect = false;
				}
				ImGui::TableNextColumn();
				ImGui::Text(u8"%s", (*iter)->IsActive ? gLangManager->GetLang("Active") : gLangManager->GetLang("Not active"));
				ImGui::TableNextColumn();
				ImGui::Text("%04d/%02d/%02d %02d:%02d:%02d", (*iter)->data_ty_block.year, (*iter)->data_ty_block.month, (*iter)->data_ty_block.day, (*iter)->data_ty_block.hour, (*iter)->data_ty_block.minute, (*iter)->data_ty_block.second);
				
				i++;
				iter++;
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
