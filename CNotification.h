#pragma once

#include "C_Time.h"
#include "imgui.h"
#include "imgui_internal.h"

class CNotification : public ITimerEvent
{
public:
	CNotification() : ShowNotification(false)
	{}

	virtual TimerResult OnTimer(void *pData)
	{
		ShowNotification = false;
		msg.clear();

		return TimerResult::Time_Stop;
	}
	virtual void OnTimerEnd(void *pData)
	{}

	void Draw();
	bool Notificatio(const char* msg, ...);
	void HelpMarcer(const char *dest);

private:
	ImGuiTextBuffer	msg;
	bool ShowNotification;
};

extern CNotification *g_pNotif;