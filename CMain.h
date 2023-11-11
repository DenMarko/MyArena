#pragma once
#include "ShowConsoleLog.h"
#include "ShowControlServer.h"
#include "C_Time.h"
#include "C_CUrl.h"
#include "CNotification.h"
#include "CListServer.h"
#include "CImGui.h"
#include "CWinWin.h"

namespace SpaceMain
{
	class CMain
	{
		class _threat 
		{
		public:
			_threat() : m_IsPendingExit(false)
			{
				m_Thread = make_shared<thread>([this]() {
					while (!m_IsPendingExit)
					{
						unique_lock<mutex> Lock(m_Mutex);
						m_CondVar.wait_for(Lock, chrono::milliseconds(10));
						timer->GlobalFrame(pTime->GetTime());
					}
				});

			}
			~_threat()
			{
				if (!m_IsPendingExit)
				{
					m_IsPendingExit = true;
					m_CondVar.notify_one();
					m_Thread->join();
				}
			}

		private:
			shared_ptr<thread> m_Thread;
			atomic<bool> m_IsPendingExit;
			mutex m_Mutex;
			condition_variable m_CondVar;
		};

		class Exception : public CBaseException
		{
		public:
			Exception(const char *file, int line, const char *note) : CBaseException(file, line, note)
			{}
			virtual std::string GetFullMessage() const override
			{
				return GetNote() + "\nAt: " + GetLocation();
			}
			virtual std::string GetExceptionType() const override
			{
				return "CMain Exception";
			}
		};
	public:
		CMain(HINSTANCE hInst, int Width, int Height);
		~CMain();

		int Loop();

	private:

		inline void PushRenderUI(const shared_ptr<CUIRender>& UI, bool *IsOpen = nullptr)
		{
			m_RenderSteck.emplace_back(UI);
			UI->OnAttach(IsOpen);
		}


	private:
		vector<shared_ptr<CUIRender>> m_RenderSteck;

		bool done;
		bool IsShowSetting;
		bool IsShowAbout;
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
		bool IsShowDebug;
#endif

		shared_ptr<CSetting> gSetting;
		shared_ptr<CControlServer> p_controlServer;
		shared_ptr<CShowConsoleLog> Showlog;
		shared_ptr<CListServer> gListServer;
		shared_ptr<SpaceWin::CWinWin> gWin;
		shared_ptr<SpaceUI::CImGui> gImGui;
		shared_ptr<Space3D::CDevice3D> gDevice3D;

		unique_ptr<_threat> th;

		VS_FIXEDFILEINFO *pFileInfo;
	};
}
