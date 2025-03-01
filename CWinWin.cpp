#include "CWinWin.h"
#include "CMain.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace SpaceWin
{
	WinProcValues mWinValue;
	size_window resiz;

	CWinWin::CWinWin(HINSTANCE hInst)
	{
		wc.cbSize = sizeof WNDCLASSEXW;
		wc.style = CS_HREDRAW|CS_VREDRAW;
		wc.lpfnWndProc = WndProc;
		wc.cbClsExtra = 0L;
		wc.cbWndExtra = 0L;
		wc.hInstance = hInst;
		wc.hCursor = nullptr;
		wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wc.lpszMenuName = nullptr;
		wc.lpszClassName = L"MyArena_beta";
		wc.hIcon = wc.hIconSm = LoadIconA(hInst, MAKEINTRESOURCE(IDI_ICON1));

		if (!RegisterClassExW(&wc))
		{
			throw Exception(__FILEW__, __LINE__, L"Failed to register window class");
		}
	}

	CWinWin::~CWinWin()
	{
		mWinValue.IsReadiWindow = false;
		this->Destroy();
	}

	void CWinWin::CreateWind(int Width, int Height)
	{
		hWind = CreateWindowW(
			wc.lpszClassName, 
			L"MyArena", 
			WS_OVERLAPPEDWINDOW, 
			(GetSystemMetrics(SM_CXSCREEN) - Width) / 2, 
			(GetSystemMetrics(SM_CYSCREEN) - Height) / 2, 
			Width, 
			Height, 
			nullptr, 
			nullptr, 
			wc.hInstance, 
			nullptr
		);
		
		if (!hWind)
		{
			mWinValue.IsReadiWindow = false;
			UnregisterClassW(wc.lpszClassName, wc.hInstance);
			throw Exception(TEXT_(__FILE__), __LINE__, L"Error Create Window");
		}

		STARTUPINFOW info;
		info.cb = sizeof STARTUPINFOW;
		GetStartupInfoW(&info);
		Show(info.wShowWindow == SW_HIDE ? SW_SHOWDEFAULT : info.wShowWindow);
		mWinValue.IsReadiWindow = true;
	}

	void CWinWin::Destroy()
	{
		if(hWind == 0)
			return;

		DestroyWindow(hWind);
		UnregisterClassW(wc.lpszClassName, wc.hInstance);
	}

	void CWinWin::setTitle(bool t)
	{
		if(mWinValue.m_TitleBar != t)
			mWinValue.m_TitleBar = t;
	}

	bool CWinWin::GetResize(size_window &siz)
	{
		if (!resiz.empty())
		{
			siz = resiz;
			resiz.reset();
			return true;
		}
		return false;
	}

	void CWinWin::SystemCommand(WPARAM wParam)
	{
		if(hWind == 0)
			return;

		SendMessage(hWind, WM_SYSCOMMAND, wParam, 0);
	}

	void CWinWin::Show(int iCmdShow)
	{
		if(hWind == 0)
			return;

		ShowWindow(hWind, iCmdShow);
		UpdateWindow(hWind);
	}

	ImGuiViewportP *FindViewportByHandleWin(HWND hWnd)
	{
		auto cont = ImGui::GetCurrentContext();
		
		for(ImGuiViewportP* v : cont->Viewports)
			if(v->PlatformHandle == hWnd)
				return v;

		return nullptr;
	}

	bool CWinWin::IsWindowActive()
	{
		HWND focuseWind = GetForegroundWindow();
		if(focuseWind == nullptr)
			return false;

		if (focuseWind == hWind)
			return true;

		if (FindViewportByHandleWin(focuseWind) != nullptr)
		{
			return true;
		}

		return false;
	}

	LRESULT WINAPI CWinWin::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		if(ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			return true;

		BOOL hasThickFrame = GetWindowLongPtrW(hWnd, GWL_STYLE) & WS_THICKFRAME;
		switch (msg)
		{
		case WM_CREATE:
		{
			if(mWinValue.IsReadiWindow)
				break;

			if (hasThickFrame)
			{
				RECT size_rect;
				GetWindowRect(hWnd, &size_rect);
				SetWindowPos(hWnd, NULL, size_rect.left, size_rect.top, size_rect.right - size_rect.left, size_rect.bottom - size_rect.top, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
			}
			break;
		}
		case WM_NCCALCSIZE:
		{
			if(!hasThickFrame || !wParam)
				break;

			const int resizeBorderX = GetSystemMetrics(SM_CXFRAME);
			const int resizeBorderY = GetSystemMetrics(SM_CYFRAME);

			NCCALCSIZE_PARAMS* params = (NCCALCSIZE_PARAMS*)lParam;
			RECT* requestedClientRect = params->rgrc;

			requestedClientRect->right = requestedClientRect->right - resizeBorderX;
			requestedClientRect->left = requestedClientRect->left + resizeBorderX;
			requestedClientRect->bottom = requestedClientRect->bottom - resizeBorderY;

			if(IsZoomed(hWnd))
				requestedClientRect->top = requestedClientRect->top + 6;
			else
				requestedClientRect->top = requestedClientRect->top + 0;

			return WVR_ALIGNTOP | WVR_ALIGNLEFT;
		}
		case WM_ACTIVATE:
		{
			RECT title_bar_rect = {0};
			InvalidateRect(hWnd, &title_bar_rect, FALSE);
			if(!mWinValue.IsReadiWindow) break;
		}
		case WM_NCHITTEST:
		{
			POINT pt = { ((int)(short)LOWORD(lParam)), ((int)(short)HIWORD(lParam)) };
			RECT rc;

			ScreenToClient(hWnd, &pt);
			GetClientRect(hWnd, &rc);

			if (!IsZoomed(hWnd))
			{
				RECT rc;
				GetClientRect(hWnd, &rc);

				enum { left = 1, top = 2, right = 4, bottom = 8 };
				int hit = 0;
				if (pt.x <= 4)
					hit |= left;
				if (pt.x >= rc.right - 4)
					hit |= right;
				if (pt.y <= 4 || pt.y < GetSystemMetrics(SM_CYFRAME))
					hit |= top;
				if (pt.y >= rc.bottom - 4)
					hit |= bottom;

				if (hit & top && hit & left)        return HTTOPLEFT;
				if (hit & top && hit & right)       return HTTOPRIGHT;
				if (hit & bottom && hit & left)     return HTBOTTOMLEFT;
				if (hit & bottom && hit & right)    return HTBOTTOMRIGHT;
				if (hit & left)                     return HTLEFT;
				if (hit & top)                      return HTTOP;
				if (hit & right)                    return HTRIGHT;
				if (hit & bottom)                   return HTBOTTOM;
			}

			if (mWinValue.m_TitleBar)
			{
				return HTCAPTION;
			}

			return HTCLIENT;
		}
		case WM_SIZE:
		{
			if (wParam != SIZE_MINIMIZED)
			{
				resiz = size_window((UINT)LOWORD(lParam), (UINT)HIWORD(lParam));
			}

			RECT size_rect;
			GetWindowRect(hWnd, &size_rect);
			SetWindowPos(hWnd, NULL, size_rect.left, size_rect.top, size_rect.right - size_rect.left, size_rect.bottom - size_rect.top, SWP_FRAMECHANGED|SWP_NOMOVE|SWP_NOSIZE);
			return 0;
		}
		case WM_SYSCOMMAND:
			switch (wParam & 0xfff0)
			{
			case SC_KEYMENU:
				return 0;
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_DPICHANGED:
			if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
			{
				const RECT* suggested_rect = (RECT*)lParam;
				SetWindowPos(hWnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
			}
			break;
		}
		return DefWindowProcW(hWnd, msg, wParam, lParam);
	}
}