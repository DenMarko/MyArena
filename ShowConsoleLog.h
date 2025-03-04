#pragma once
#include "imgui.h"
#include <ctype.h>
#include <stdlib.h>
#include "C_Time.h"
#include "C_CUrl.h"

class CShowConsoleLog : public ITimerEvent, public CUIRender
{
public:
	CShowConsoleLog()
	{
		memset(InputBuf, 0, sizeof(InputBuf));
		HistoryPos = -1;
		Commands.push_back("LIMIT");
		Clear();
	}

	~CShowConsoleLog() 
	{
		Clear();
		for (int i = 0; i > History.Size; i++)
		{
			mem::Free( History[i]);
		}
	}

	void Clear();

	void AddLog(const char *fmt, ...);

	virtual void OnAttach(bool *IsOpen) override;
	virtual void OnDetach() override {}

	virtual void OnUIRender() override;

	virtual TimerResult OnTimer(void *pData) override;
	virtual void OnTimerEnd(void *pData) override {}

private:
	static int	TextEditCallbackStub(ImGuiInputTextCallbackData* data)
	{
		CShowConsoleLog* console = (CShowConsoleLog*)data->UserData;
		return console->TextEditCallback(data);
	}

	void CheckLimit(ImGuiTextBuffer &pBuf, ImVector<int> &pLineOffset);
	void ExecCmd(const char *cmd);

private:
	static int		Stricmp(const char* s1, const char* s2)         { int d; while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; } return d; }
	static int		Strnicmp(const char* s1, const char* s2, int n) { int d = 0; while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; n--; } return d; }
	static char*	Strdup(const char* s)                           { IM_ASSERT(s); size_t len = strlen(s) + 1; void* buf = mem::Malloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)s, len); }
	static void		Strtrim(char* s)                                { char* str_end = s + strlen(s); while (str_end > s && str_end[-1] == ' ') str_end--; *str_end = 0; }

	int				TextEditCallback(ImGuiInputTextCallbackData* data);

	ImGuiTextBuffer			lBuf;
	ImVector<int>			LineOffsets;
	ImVector<char*>			History;
	ImVector<const char*>	Commands;
	char					InputBuf[256];
	int						HistoryPos;
};