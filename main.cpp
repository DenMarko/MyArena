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
#include "resource.h"

static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

//bool m_TitleBarHovered = false;

vector<CUIRender*> m_RenderSteck;

void PushRenderUI(CUIRender* UI, bool *IsOpen);

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

class _threat 
{
public:
	_threat(shared_ptr<C_Timer *> p_Timer) : m_IsPendingExit(false), pTimer(p_Timer)
	{
		m_Thread = make_shared<thread>([this]() {
			while (!m_IsPendingExit)
			{
				unique_lock<mutex> Lock(m_Mutex);
				m_CondVar.wait_for(Lock, chrono::milliseconds(10));
				(*pTimer)->GlobalFrame(pTime->GetTime());
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
	shared_ptr<C_Timer *> pTimer;
	atomic<bool> m_IsPendingExit;
	mutex m_Mutex;
	condition_variable m_CondVar;
};

void RenderWindowOuterBorders(ImGuiWindow* window)
{
	struct ImGuiResizeBorderDef
	{
		ImVec2 InnerDir;
		ImVec2 SegmentN1, SegmentN2;
		float  OuterAngle;
	};

	static const ImGuiResizeBorderDef resize_border_def[4] =
	{
		{ ImVec2(+1, 0), ImVec2(0, 1), ImVec2(0, 0), IM_PI * 1.00f }, // Left
	{ ImVec2(-1, 0), ImVec2(1, 0), ImVec2(1, 1), IM_PI * 0.00f }, // Right
	{ ImVec2(0, +1), ImVec2(0, 0), ImVec2(1, 0), IM_PI * 1.50f }, // Up
	{ ImVec2(0, -1), ImVec2(1, 1), ImVec2(0, 1), IM_PI * 0.50f }  // Down
	};

	auto GetResizeBorderRect = [](ImGuiWindow* window, int border_n, float perp_padding, float thickness)
	{
		ImRect rect = window->Rect();
		if (thickness == 0.0f)
		{
			rect.Max.x -= 1;
			rect.Max.y -= 1;
		}
		if (border_n == ImGuiDir_Left) { return ImRect(rect.Min.x - thickness, rect.Min.y + perp_padding, rect.Min.x + thickness, rect.Max.y - perp_padding); }
		if (border_n == ImGuiDir_Right) { return ImRect(rect.Max.x - thickness, rect.Min.y + perp_padding, rect.Max.x + thickness, rect.Max.y - perp_padding); }
		if (border_n == ImGuiDir_Up) { return ImRect(rect.Min.x + perp_padding, rect.Min.y - thickness, rect.Max.x - perp_padding, rect.Min.y + thickness); }
		if (border_n == ImGuiDir_Down) { return ImRect(rect.Min.x + perp_padding, rect.Max.y - thickness, rect.Max.x - perp_padding, rect.Max.y + thickness); }
		IM_ASSERT(0);
		return ImRect();
	};


	ImGuiContext& g = *GImGui;
	float rounding = window->WindowRounding;
	float border_size = 1.0f; // window->WindowBorderSize;
	if (border_size > 0.0f && !(window->Flags & ImGuiWindowFlags_NoBackground))
		window->DrawList->AddRect(window->Pos, { window->Pos.x + window->Size.x,  window->Pos.y + window->Size.y }, ImGui::GetColorU32(ImGuiCol_Border), rounding, 0, border_size);

	int border_held = window->ResizeBorderHeld;
	if (border_held != -1)
	{
		const ImGuiResizeBorderDef& def = resize_border_def[border_held];
		ImRect border_r = GetResizeBorderRect(window, border_held, rounding, 0.0f);
		ImVec2 p1 = ImLerp(border_r.Min, border_r.Max, def.SegmentN1);
		const float offsetX = def.InnerDir.x * rounding;
		const float offsetY = def.InnerDir.y * rounding;
		p1.x += 0.5f + offsetX;
		p1.y += 0.5f + offsetY;

		ImVec2 p2 = ImLerp(border_r.Min, border_r.Max, def.SegmentN2);
		p2.x += 0.5f + offsetX;
		p2.y += 0.5f + offsetY;

		window->DrawList->PathArcTo(p1, rounding, def.OuterAngle - IM_PI * 0.25f, def.OuterAngle);
		window->DrawList->PathArcTo(p2, rounding, def.OuterAngle, def.OuterAngle + IM_PI * 0.25f);
		window->DrawList->PathStroke(ImGui::GetColorU32(ImGuiCol_SeparatorActive), 0, ImMax(2.0f, border_size)); // Thicker than usual
	}
	if (g.Style.FrameBorderSize > 0 && !(window->Flags & ImGuiWindowFlags_NoTitleBar) && !window->DockIsActive)
	{
		float y = window->Pos.y + window->TitleBarHeight() - 1;
		window->DrawList->AddLine(ImVec2(window->Pos.x + border_size, y), ImVec2(window->Pos.x + window->Size.x - border_size, y), ImGui::GetColorU32(ImGuiCol_Border), g.Style.FrameBorderSize);
	}
}


int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCommandLine, int nCommandShow)
{
	WNDCLASSEXW wc = {	sizeof(wc), 
						CS_CLASSDC, 
						WndProc, 
						0L, 
						0L, 
						hInst, 
						LoadIconA(hInst, MAKEINTRESOURCE(IDI_ICON1)), 
						nullptr, 
						nullptr, 
						nullptr, 
						L"MyArena_beta", 
						LoadIconA(hInst, MAKEINTRESOURCE(IDI_ICON1))
						};

	VS_FIXEDFILEINFO *pFileInfo = nullptr;
	HRSRC hRes = FindResource(hInst, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
	if (hRes != nullptr)
	{
		HGLOBAL hResGlob = LoadResource(hInst, hRes);
		if (hResGlob != nullptr)
		{
			void* pData = LockResource(hResGlob);
			if (pData != nullptr)
			{
				UINT len;
				VerQueryValue(pData, "\\", (void**)&pFileInfo, &len);
			}
			FreeResource(hResGlob);
		}
	}

	RegisterClassExW(&wc);
	HWND hWind = CreateWindowW(wc.lpszClassName, L"MyArena", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

	if (!CreateDeviceD3D(hWind))
	{
		CleanupDeviceD3D();
		UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return -1;
	}

	ShowWindow(hWind, nCommandShow);
	UpdateWindow(hWind);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	g_pGlob->pIO = &ImGui::GetIO();
	g_pGlob->pIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	g_pGlob->pIO->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	g_pGlob->pIO->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	g_pGlob->pIO->ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;

	//g_pGlob->pIO->ConfigViewportsNoAutoMerge = true;
	g_pGlob->pIO->ConfigViewportsNoTaskBarIcon = true;
	g_pGlob->pIO->ConfigDockingTransparentPayload = true;

	CSetting *gSetting = new CSetting();
	gSetting->SetStile();

	ImGui_ImplWin32_Init(hWind);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	g_pGlob->pFont = g_pGlob->pIO->Fonts->AddFontFromFileTTF("C:/Windows/fonts/consola.ttf", g_pGlob->fFontSize, nullptr, g_pGlob->pIO->Fonts->GetGlyphRangesCyrillic());
	g_pGlob->pIO->Fonts->Build();

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	CControlServer *p_controlServer = new CControlServer(g_pd3dDevice, pUrls);
	CShowConsoleLog *Showlog = new CShowConsoleLog(pUrls);

	_threat *th = new _threat(std::make_shared<C_Timer *>(timer));

	g_pGlob->g_ServerConsole = timer->CreateTimer(Showlog,			g_pGlob->fIntervalServerConsole, nullptr);
	g_pGlob->g_ControlServer = timer->CreateTimer(p_controlServer,	g_pGlob->fIntervalControlServer, nullptr);

	bool done = false;
	bool IsShowConsoleLog = true;
	bool IsShowControlServer = true;
	bool IsShowSetting = false;
	bool IsShowAbout = false;
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
	bool IsShowDebug = true;
#endif

	PushRenderUI(Showlog,			&IsShowConsoleLog);
	PushRenderUI(p_controlServer,	&IsShowControlServer);
	PushRenderUI(gSetting,			&IsShowSetting);
	PushRenderUI(g_pNotif,			nullptr);
	PushRenderUI(pUrls,				nullptr);

	while (!done)
	{
		MSG msg;
		while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if(msg.message == WM_QUIT)
				done = true;
		}

		if(done)
			break;

		if (g_ResizeHeight != 0 && g_ResizeWidth != 0)
		{
			CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
			g_ResizeWidth = g_ResizeHeight = 0;
			CreateRenderTarget();
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		{
			ImGuiWindowFlags win_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_MenuBar;
			const ImGuiViewport* viewport = ImGui::GetMainViewport();

			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

			win_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			win_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

			const bool isMaximized = IsZoomed(hWind);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, isMaximized ? ImVec2(6.f, 6.f) : ImVec2(1.f, 1.f));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);

			ImGui::Begin("DockSpace Demo", nullptr, win_flags);
			ImGui::PopStyleVar(2);
			ImGui::PopStyleVar(2);

			{
				ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(50, 50, 50, 255));

				if(!isMaximized)
					RenderWindowOuterBorders(ImGui::GetCurrentWindow());

				ImGui::PopStyleColor();
			}

			//m_TitleBarHovered = ImGui::IsItemHovered();

			//if (isMaximized)
			//{
			//	float winMousePosY = ImGui::GetMousePos().y - ImGui::GetCursorScreenPos().y;
			//	if (winMousePosY >= 0.f && winMousePosY <= 5.f)
			//	{
			//		m_TitleBarHovered = true;
			//	}
			//}

			if (g_pGlob->pIO->ConfigFlags & ImGuiConfigFlags_DockingEnable)
			{
				ImGui::DockSpace(ImGui::GetID("DirectXAppDockspace"));
			}

			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu(gLangManager->GetLang("File")))
				{
					if (ImGui::MenuItem(gLangManager->GetLang("Server console"), NULL, IsShowConsoleLog)) { IsShowConsoleLog = IsShowConsoleLog ? false : true; }
					if (ImGui::MenuItem(gLangManager->GetLang("Server management"), NULL, IsShowControlServer)) { IsShowControlServer = IsShowControlServer ? false : true; }
					if (ImGui::MenuItem(gLangManager->GetLang("Settings"), NULL, IsShowSetting)) { IsShowSetting = IsShowSetting ? false : true; }
					if (ImGui::MenuItem(gLangManager->GetLang("Exit"))) { done = true; }

					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu(gLangManager->GetLang("Help")))
				{
					if (ImGui::MenuItem(gLangManager->GetLang("About"), NULL, IsShowAbout)) { IsShowAbout = IsShowAbout ? false : true; }

					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			if (IsShowAbout)
			{
				ImVec2 vCenter = ImGui::GetMainViewport()->GetCenter();
				ImGui::SetNextWindowPos(vCenter, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
				ImGui::SetNextWindowSize(ImVec2(300, 200), ImGuiCond_Always);

				ImGui::Begin(gLangManager->GetLang("About"), &IsShowAbout);

				ImGui::Text(gLangManager->GetLang("Version of the components used in the program:"));

				ImGui::Text("ImGui ver: %s", IMGUI_VERSION);
				ImGui::Text("CURL  ver: %s", LIBCURL_VERSION);
				ImGui::Text("RAPIDJSON: %s", RAPIDJSON_VERSION_STRING);
				if(pFileInfo != nullptr)
					ImGui::Text("Version program: %d.%d.%d.%d", HIWORD(pFileInfo->dwFileVersionMS), LOWORD(pFileInfo->dwFileVersionMS), HIWORD(pFileInfo->dwFileVersionLS), LOWORD(pFileInfo->dwFileVersionLS));

				ImGui::SetCursorPos(ImVec2((300.f - 80.f) * 0.5f, (200.f + 100.f) * 0.5f));
				if(ImGui::Button("Ok", ImVec2(80, 0)))
					IsShowAbout = false;
				ImGui::End();
			}

#ifndef IMGUI_DISABLE_DEBUG_TOOLS
			if(IsShowDebug)
				ImGui::ShowDebugLogWindow(&IsShowDebug);
#endif
			for(auto UI_Render : m_RenderSteck)
				UI_Render->OnUIRender();
			
			ImGui::End();
		}

		ImGui::Render();
		const float clear_color_with_alpha[4] = {clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w};
	
		g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
		g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);

		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		if (g_pGlob->pIO->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		g_pSwapChain->Present(1, 0);
	}

	for(auto UI_Render : m_RenderSteck)
		UI_Render->OnDetach();

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	DestroyWindow(hWind);
	UnregisterClassW(wc.lpszClassName, wc.hInstance);

	delete gSetting;
	delete th;
	delete timer;
	delete Showlog;
	delete p_controlServer;

	return 0;
}

bool CreateDeviceD3D(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res == DXGI_ERROR_UNSUPPORTED)
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}


void PushRenderUI(CUIRender* UI, bool *IsOpen)
{
	m_RenderSteck.emplace_back(UI);
	UI->OnAttach(IsOpen);
}

//#include <windowsx.h>
//#include <uxtheme.h>
//#include <dwmapi.h>

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//static RECT border_thickness = { 4, 4, 4, 4 };
	//BOOL hasThickFrame = GetWindowLongPtr(hWnd, GWL_STYLE) & WS_THICKFRAME;

	if(ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	//case WM_CREATE:
	//{
	//	if (hasThickFrame)
	//	{
	//		RECT size_rect;
	//		GetWindowRect(hWnd, &size_rect);

	//		// Inform the application of the frame change to force redrawing with the new
	//		// client area that is extended into the title bar
	//		SetWindowPos(
	//			hWnd, NULL,
	//			size_rect.left, size_rect.top,
	//			size_rect.right - size_rect.left, size_rect.bottom - size_rect.top,
	//			SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE
	//		);
	//		break;
	//	}

	//	break;
	//}
	//case WM_NCCALCSIZE:
	//{
	//	if(!wParam || !hasThickFrame)
	//		break;

	//	const int resizeBorderX = GetSystemMetrics(SM_CXFRAME);
	//	const int resizeBorderY = GetSystemMetrics(SM_CYFRAME);

	//	NCCALCSIZE_PARAMS *params = (NCCALCSIZE_PARAMS*)lParam;
	//	RECT *rectClient = params->rgrc;

	//	rectClient->right -= resizeBorderX;
	//	rectClient->left += resizeBorderX;
	//	rectClient->bottom -= resizeBorderY;
	//	rectClient->top += 0;

	//	return WVR_ALIGNTOP | WVR_ALIGNLEFT;
	//}
	//case WM_NCHITTEST:
	//{
	//	if(!hasThickFrame)
	//		break;

	//	POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
	//	ScreenToClient(hWnd, &pt);

	//	if(!IsZoomed(hWnd))
	//	{
	//		RECT rc;
	//		GetClientRect(hWnd, &rc);

	//		const int verticBordenSize = GetSystemMetrics(SM_CYFRAME);

	//		enum { left = 1, top = 2, right = 4, bottom = 8 };
	//		int hit = 0;
	//		if (pt.x <= border_thickness.left)								hit |= left;
	//		if (pt.x >= rc.right - border_thickness.right)					hit |= right;
	//		if (pt.y <= border_thickness.top || pt.y < verticBordenSize)	hit |= top;
	//		if (pt.y >= rc.bottom - border_thickness.bottom)				hit |= bottom;

	//		if (hit & top && hit & left)        return HTTOPLEFT;
	//		if (hit & top && hit & right)       return HTTOPRIGHT;
	//		if (hit & bottom && hit & left)     return HTBOTTOMLEFT;
	//		if (hit & bottom && hit & right)    return HTBOTTOMRIGHT;
	//		if (hit & left)                     return HTLEFT;
	//		if (hit & top)                      return HTTOP;
	//		if (hit & right)                    return HTRIGHT;
	//		if (hit & bottom)                   return HTBOTTOM;
	//	}

	//	if(m_TitleBarHovered)
	//		return HTCAPTION;

	//	return HTCLIENT;
	//}
	case WM_SIZE:
		if(wParam == SIZE_MINIMIZED)
			return 0;

		g_ResizeWidth = (UINT)LOWORD(lParam);
		g_ResizeHeight = (UINT)HIWORD(lParam);
		return 0;
	case WM_SYSCOMMAND:
		if((wParam & 0xfff) == SC_KEYMENU)
			return 0;
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