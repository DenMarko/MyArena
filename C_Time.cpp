#include "C_Time.h"
#include "imgui_internal.h"

using namespace my_timer;

C_Time mTime;
C_Time *pTime = &mTime;

C_Time::C_Time()
{
	QueryPerformanceFrequency(&pPerformFreq);
	QueryPerformanceCounter(&pClockStart);
}

double C_Time::GetTime()
{
	LARGE_INTEGER CurrentTime;
	QueryPerformanceCounter(&CurrentTime);

	return static_cast<double>(CurrentTime.QuadPart - pClockStart.QuadPart) / static_cast<double>(pPerformFreq.QuadPart);
}

#define TIMER_MIN_ACCURACY		0.1f

void ITimer::Initialize(const std::shared_ptr<ITimerEvent> &pCallBack, float fInterval, float fToExec, void * pData)
{
	mListener = pCallBack;
	mInterval = fInterval;
	fSetNewInterval = 0;
	mToExec = fToExec;
	m_pData = pData;
	mInExec = false;
	mKillMe = false;
}

std::unique_ptr<C_Timer> timer(std::make_unique<C_Timer>());
C_Timer::C_Timer() : m_fLastTickedTime(0.f), g_fTimerThink(0.f), g_fUniversalTime(0.f)
{
}

C_Timer::~C_Timer()
{
	std::lock_guard<std::mutex> Lock(mMutex);
	for (auto iter = mLoopTimer.begin(); iter != mLoopTimer.end(); iter++)
	{
		m_delete(*iter);
	}
	mLoopTimer.clear();
}

void C_Timer::GlobalFrame(double times)
{
	{
		g_fUniversalTime += (static_cast<float>(times) - m_fLastTickedTime);
		m_fLastTickedTime = static_cast<float>(times);

		if (g_fUniversalTime >= g_fTimerThink)
		{
			RunFrame();
			g_fTimerThink = this->CalcNextThink(g_fTimerThink, TIMER_MIN_ACCURACY);
		}
	}

	{
		scoped_lock<std::mutex> lock(m_EventQueueMutex);
		while (m_EventQueue.size() > 0)
		{
			auto& func = m_EventQueue.front();
			func();
			m_EventQueue.pop();
		}
	}
}

void C_Timer::RunFrame()
{
	ITimer *pTimer;
	double curtime = getSimulateTime();

	TimerResult res;
	std::unique_lock<std::mutex> Lock(mMutex);
	for (auto iter = mLoopTimer.begin(); iter != mLoopTimer.end(); )
	{
		pTimer = (*iter);
		if (curtime >= pTimer->mToExec)
		{
			pTimer->mInExec = true;
			res = pTimer->mListener->OnTimer(pTimer->m_pData);
			if (pTimer->mKillMe || (res == Time_Stop))
			{
				pTimer->mListener->OnTimerEnd(pTimer->m_pData);
				iter = mLoopTimer.erase(iter);
				m_delete( pTimer);
				continue;
			}
			pTimer->mInExec = false;
			if (pTimer->fSetNewInterval)
			{
				IMGUI_DEBUG_LOG("old interval: %f\nnew interval: %f\n", pTimer->mInterval, pTimer->fSetNewInterval);
				if(pTimer->mInterval != pTimer->fSetNewInterval)
					pTimer->mInterval = pTimer->fSetNewInterval;

				pTimer->fSetNewInterval = 0.f;
			}
			pTimer->mToExec = CalcNextThink(pTimer->mToExec, pTimer->mInterval);
		}

		iter++;
	}
}

ITimer *C_Timer::CreateTimer(const std::shared_ptr<ITimerEvent> &pCallBack, float fInterval, void * pData)
{
	std::unique_lock<std::mutex> Lock(mMutex);
	ITimer *pTimer = m_new<ITimer>();
	float to_exec = static_cast<float>(getSimulateTime()) + fInterval;

	pTimer->Initialize(pCallBack, fInterval, to_exec, pData);

	mLoopTimer.push_back(pTimer);

	return pTimer;
}

void C_Timer::KillTimer(ITimer * pTimer)
{
	std::unique_lock<std::mutex> Lock(mMutex);
	if (pTimer->mKillMe)
		return;

	if (pTimer->mInExec)
	{
		pTimer->mKillMe = true;
		return;
	}


	pTimer->mInExec = true;
	pTimer->mListener->OnTimerEnd(pTimer->m_pData);

	for (auto iter = mLoopTimer.begin(); iter != mLoopTimer.end(); iter++)
	{
		if (pTimer == (*iter))
		{
			iter = mLoopTimer.erase(iter);
			m_delete( pTimer);
			break;
		}
	}
}