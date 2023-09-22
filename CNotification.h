#pragma once

#include "C_Time.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "CSetting.h"

class CNotification : public ITimerEvent,
						public CUIRender
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

	virtual void OnAttach(bool *IsOpen) override {}
	virtual void OnDetach() override {}

	virtual void OnUIRender() override;

	bool Notificatio(const char* msg, ...);
	void HelpMarcer(const char *dest);

private:
	ImGuiTextBuffer	msg;
	bool ShowNotification;
};

extern CNotification *g_pNotif;