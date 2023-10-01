#include "CNotification.h"

shared_ptr<CNotification> g_pNotif(make_shared<CNotification>());

void CNotification::OnUIRender()
{
	if (ShowNotification)
	{
		ImVec2 textSize = ImGui::CalcTextSize(msg.c_str());
		ImVec2 winSize(300, 100);

		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(winSize, ImGuiCond_Always);

		ImGui::Begin("Notification", &ShowNotification, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

		ImGui::SetCursorPos(ImVec2((winSize.x - (winSize.x - 20.f)) * 0.5f, (winSize.y - textSize.y) * 0.5f));

		ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + (winSize.x - 20.f));
		ImGui::Text(u8"%s", msg.c_str());
		ImGui::PopTextWrapPos();

		ImGui::End();
	}
}

bool CNotification::Notificatio(const char *cMsg, ...)
{
	if(!ShowNotification)
	{
		va_list args;
		va_start(args, cMsg);
		msg.appendfv(cMsg, args);
		va_end(args);

		ShowNotification = true;
		timer->CreateTimer(g_pNotif, 3.f, nullptr);
	}
	return true;
}

void CNotification::HelpMarcer(const char * dest)
{
	ImGui::TextDisabled("(?)");
	if (ImGui::BeginItemTooltip())
	{
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(dest);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

