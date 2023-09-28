#pragma once
#include <Windows.h>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

class C_Time
{
public:
	C_Time();

	double GetTime();

private:
	LARGE_INTEGER pPerformFreq;
	LARGE_INTEGER pClockStart;
};

extern C_Time *pTime;

enum TimerResult
{
	Time_Continue = 0,
	Time_Changed,
	Time_Handled,
	Time_Stop,
};

class ITimerEvent
{
public:
	virtual TimerResult OnTimer(void *pData) = 0;
	virtual void OnTimerEnd(void *pData) = 0;
};

class ITimer 
{
public:
	void Initialize(ITimerEvent *pCallBack, float fInterval, float fToExec, void *pData);
	ITimerEvent *mListener;
	void *m_pData;
	float mInterval;
	float fSetNewInterval;
	double mToExec;
	bool mInExec;
	bool mKillMe;

	void SetNewInterval(float fInterval) { fSetNewInterval = fInterval; }
};

class C_Timer
{
public:
	C_Timer();
	~C_Timer();

public:
	void GlobalFrame(double times);
	ITimer *CreateTimer(ITimerEvent* pCallBack, float fInterval, void *pData);
	void KillTimer(ITimer* pTimer);
private:
	void RunFrame();
	inline double CalcNextThink(double last, float interval)
	{
		if (g_fUniversalTime - last - interval <= 0.1)
			return last + interval;
		else
			return g_fUniversalTime + interval;
	}
	inline double getSimulateTime()
	{
		return g_fUniversalTime;
	}

	double g_fUniversalTime;
	double g_fTimerThink;
	float m_fLastTickedTime;

	std::vector<ITimer *>	mLoopTimer;
	std::mutex				mMutex;
};

extern C_Timer *timer;