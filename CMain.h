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
	class _threat 
	{
	public:
		_threat();
		~_threat();

	private:
		shared_ptr<thread> m_Thread;
		atomic<bool> m_IsPendingExit;
		mutex m_Mutex;
		condition_variable m_CondVar;
	};

	class CMain
	{

		class Exception : public CBaseException
		{
		public:
			Exception(const wchar_t *file, int line, const wchar_t *note) : CBaseException(file, line, note)
			{}
			virtual std::wstring GetFullMessage() const override
			{
				return GetNote() + L"\nAt: " + GetLocation();
			}
			virtual std::wstring GetExceptionType() const override
			{
				return std::wstring(L"CMain Exception");
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

		void GetResourceInfo(HINSTANCE hInst);
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
		string pVersionProg;
	};
}
