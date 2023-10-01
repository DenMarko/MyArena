#pragma once
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include <d3d11.h>
#include "ShowConsoleLog.h"
#include "ShowControlServer.h"
#include "C_Time.h"
#include "C_CUrl.h"
#include "CNotification.h"
#include "CSetting.h"
#include "CException.h"
#include "resource.h"

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
				return "Windows Exception";
			}
		};
	public:
		CMain(HINSTANCE hInst, int Width, int Height);
		~CMain();

		int Loop();

	private:
		void RenderWindowOuterBorders(ImGuiWindow* window);
		bool IsMouseHovered(const ImVec2& MousePos, const ImVec2& pos_min, const ImVec2& pos_max);
		bool IsCloseButtomDown(const ImVec2& MousePos, const ImVec2& pos_min, const ImVec2& pos_max);
		bool IsMinimiseButtomDown(const ImVec2& MousePos, const ImVec2& pos_min, const ImVec2& pos_max);
		bool IsMaximiseButtomDown(const ImVec2& MousePos, const ImVec2& pos_min, const ImVec2& pos_max);
		bool CreateDeviceD3D(HWND hWnd);
		void CleanupDeviceD3D();
		void CreateRenderTarget();
		void CleanupRenderTarget();
		static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		void PushRenderUI(const shared_ptr<CUIRender>& UI, bool *IsOpen);

	private:
		HWND hWind;
		WNDCLASSEXW wc;
		ImVec4 clear_color;
		vector<shared_ptr<CUIRender>> m_RenderSteck;


		ID3D11Device*            g_pd3dDevice;
		ID3D11DeviceContext*     g_pd3dDeviceContext;
		IDXGISwapChain*          g_pSwapChain;
		ID3D11RenderTargetView*  g_mainRenderTargetView;

		bool done;
		bool IsShowConsoleLog;
		bool IsShowControlServer;
		bool IsShowSetting;
		bool IsShowAbout;
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
		bool IsShowDebug;
#endif

		shared_ptr<CSetting> gSetting;
		shared_ptr<CControlServer> p_controlServer;
		shared_ptr<CShowConsoleLog> Showlog;
		unique_ptr<_threat> th;

		VS_FIXEDFILEINFO *pFileInfo;
	};
}
