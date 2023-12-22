#pragma once
#include <Windows.h>
#include <vector>
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include "CMemory.h"

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
	virtual ~ITimerEvent() = default;

	virtual TimerResult OnTimer(void *pData) = 0;
	virtual void OnTimerEnd(void *pData) = 0;
};

class ITimer
{
public:
	void Initialize(const std::shared_ptr<ITimerEvent> &pCallBack, float fInterval, float fToExec, void *pData);
	std::shared_ptr<ITimerEvent> mListener;
	void *m_pData;
	float mInterval;
	float fSetNewInterval;
	double mToExec;
	bool mInExec;
	bool mKillMe;

	void SetNewInterval(float fInterval) { fSetNewInterval = fInterval; }
};

namespace my_timer
{
	class C_Timer
	{
	public:
		C_Timer();
		~C_Timer();

	public:
		void GlobalFrame(double times);
		ITimer *CreateTimer(const std::shared_ptr<ITimerEvent> &pCallBack, float fInterval, void *pData);
		void KillTimer(ITimer* pTimer);

		template<typename F> void AddNextFrame(F&& func)
		{
			m_EventQueue.push(func);
		}
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

		std::vector<ITimer*>				mLoopTimer;
		std::mutex							mMutex;

		std::mutex							m_EventQueueMutex;
		std::queue<std::function<void()>>	m_EventQueue;
	};

	template<class... _Mutexes>
	class scoped_lock
	{
	public:
		explicit scoped_lock(_Mutexes&... _Mtxes)
			: _MyMutexes(_Mtxes...)
		{
			_STD lock(_Mtxes...);
		}

		explicit scoped_lock(std::adopt_lock_t, _Mutexes&... _Mtxes)
			: _MyMutexes(_Mtxes...)
		{
		}

		~scoped_lock() noexcept
		{
			_For_each_tuple_element(
				_MyMutexes,
				[](auto& _Mutex) noexcept { _Mutex.unlock(); });
		}

		scoped_lock(const scoped_lock&) = delete;
		scoped_lock& operator=(const scoped_lock&) = delete;
	private:
		std::tuple<_Mutexes&...> _MyMutexes;
	};

	template<class _Mutex>
	class scoped_lock<_Mutex>
	{
	public:
		typedef _Mutex mutex_type;

		explicit scoped_lock(_Mutex& _Mtx)
			: _MyMutex(_Mtx)
		{
			_MyMutex.lock();
		}

		explicit scoped_lock(std::adopt_lock_t, _Mutex& _Mtx)
			: _MyMutex(_Mtx)
		{
		}

		~scoped_lock() noexcept
		{
			_MyMutex.unlock();
		}

		scoped_lock(const scoped_lock&) = delete;
		scoped_lock& operator=(const scoped_lock&) = delete;
	private:
		_Mutex& _MyMutex;
	};
}

extern std::unique_ptr<my_timer::C_Timer> timer;