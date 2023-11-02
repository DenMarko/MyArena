#include "ShowConsoleLog.h"
#include "CNotification.h"

#pragma warning(disable: 4996)

void CShowConsoleLog::Clear()
{
	lBuf.clear();
	LineOffsets.clear();
	LineOffsets.push_back(0);
}

void CShowConsoleLog::AddLog(const char *fmt, ...)
{
	int old_size = lBuf.size();
	va_list args;

	va_start(args, fmt);
	lBuf.appendfv(fmt, args);
	va_end(args);

	for (int new_size = lBuf.size(); old_size < new_size; old_size++) {
		if (lBuf[old_size] == '\n') {
			LineOffsets.push_back(old_size + 1);
		}
	}
	CheckLimit(lBuf, LineOffsets);
}

void CShowConsoleLog::OnAttach(bool * IsOpen) {}
void CShowConsoleLog::OnUIRender()
{
	if (g_pGlob->IsShowConsoleLog)
	{
		ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
		if (!ImGui::Begin(gLangManager->GetLang("Server console"), &g_pGlob->IsShowConsoleLog))
		{
			ImGui::End();
			return;
		}

		const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
		if (ImGui::BeginChild("scrolling", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

			const char *buf = lBuf.begin();
			const char *buf_end = lBuf.end();

			ImGuiListClipper cliper;
			cliper.Begin(LineOffsets.Size);

			while (cliper.Step())
			{
				for (int line_no = cliper.DisplayStart; line_no < cliper.DisplayEnd; line_no++)
				{
					const char *line_start = buf + LineOffsets[line_no];
					const char *line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
					ImGui::TextUnformatted(line_start, line_end);
				}
			}

			cliper.End();
			ImGui::PopStyleVar();

			if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			{
				ImGui::SetScrollHereY(1.0f);
			}
		}

		ImGui::EndChild();
		ImGui::Separator();

		ImGuiInputTextFlags fInput_text = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
		if (ImGui::InputText(gLangManager->GetLang("Enter the command"), InputBuf, IM_ARRAYSIZE(InputBuf), fInput_text, &TextEditCallbackStub, (void*)this))
		{
			char *s = InputBuf;
			Strtrim(s);
			if (s[0]) {
				CmdResult* res = reinterpret_cast< CmdResult *>(pUrls->GetData(CONSOLE_CMD, s));
				if (res->status == OK)
				{
					g_pNotif->Notificatio(res->msg.c_str());
				}
				delete res;

				HistoryPos = -1;
				for (int i = History.Size - 1; i >= 0; i--)
				{
					if (Stricmp(History[i], s) == 0)
					{
						free(History[i]);
						History.erase(History.begin() + i);
						break;
					}
				}
				History.push_back(Strdup(s));
			}
			strcpy(s, "");
		}
		ImGui::SetItemDefaultFocus();

		ImGui::End();

	}
	return;
}

int CShowConsoleLog::TextEditCallback(ImGuiInputTextCallbackData* data)
{
	switch (data->EventFlag)
	{
		case ImGuiInputTextFlags_CallbackCompletion:
		{
			const char* word_end = data->Buf + data->CursorPos;
			const char* word_start = word_end;
			while (word_start > data->Buf)
			{
				const char c = word_start[-1];
				if (c == ' ' || c == '\t' || c == ',' || c == ';')
					break;
				word_start--;
			}

			ImVector<const char*> candidates;
			candidates.push_back(word_start);

			if (candidates.Size == 1)
			{
				data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
				data->InsertChars(data->CursorPos, candidates[0]);
				data->InsertChars(data->CursorPos, " ");
			}
			else
			{
				int match_len = (int)(word_end - word_start);
				for (;;)
				{
					int c = 0;
					bool all_candidates_matches = true;
					for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
						if (i == 0)
							c = toupper(candidates[i][match_len]);
						else if (c == 0 || c != toupper(candidates[i][match_len]))
							all_candidates_matches = false;
					if (!all_candidates_matches)
						break;
					match_len++;
				}

				if (match_len > 0)
				{
					data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
					data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
				}

				AddLog("Possible matches:\n");
				for (int i = 0; i < candidates.Size; i++)
					AddLog("- %s\n", candidates[i]);
			}

			break;
		}
		case ImGuiInputTextFlags_CallbackHistory:
		{
			const int prev_history_pos = HistoryPos;
			if (data->EventKey == ImGuiKey_UpArrow)
			{
				if (HistoryPos == -1)
					HistoryPos = History.Size - 1;
				else if (HistoryPos > 0)
					HistoryPos--;
			}
			else if (data->EventKey == ImGuiKey_DownArrow)
			{
				if (HistoryPos != -1)
					if (++HistoryPos >= History.Size)
						HistoryPos = -1;
			}

			if (prev_history_pos != HistoryPos)
			{
				const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
				data->DeleteChars(0, data->BufTextLen);
				data->InsertChars(0, history_str);
			}
		}
	}
	return 0;
}


TimerResult CShowConsoleLog::OnTimer(void *pData)
{
	auto *con_log = reinterpret_cast<GetConsole *>(pUrls->GetData(GET_CONSOLE));

	if(con_log == nullptr)
		return Time_Continue;

	if (con_log->s == OK)
	{
		if(con_log->console_log)
			AddLog("%s", con_log->console_log);
	}

	delete con_log;
	return TimerResult::Time_Continue;
}