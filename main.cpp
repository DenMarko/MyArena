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

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	g_pGlob->pIO = &io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	CSetting *gSetting = new CSetting();

	gSetting->SetStile();

	ImGui_ImplWin32_Init(hWind);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	g_pGlob->pFont = io.Fonts->AddFontFromFileTTF("C:/Windows/fonts/consola.ttf", g_pGlob->fFontSize, nullptr, io.Fonts->GetGlyphRangesCyrillic());
	io.Fonts->Build();

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	CControlServer *p_controlServer = new CControlServer(g_pd3dDevice, pUrls);
	_threat *th = new _threat(std::make_shared<C_Timer *>(timer));

	g_pGlob->g_ServerConsole = timer->CreateTimer(Showlog,			g_pGlob->fIntervalServerConsole, pUrls);
	g_pGlob->g_ControlServer = timer->CreateTimer(p_controlServer,	g_pGlob->fIntervalControlServer, nullptr);

	bool done = false;
	bool IsShowConsoleLog = true;
	bool IsShowControlServer = true;
	bool IsShowSetting = false;
	bool IsShowAbout = false;
#ifndef IMGUI_DISABLE_DEBUG_TOOLS
	bool IsShowDebug = true;
#endif

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

		/** * * тут пишим нові фрейми * **/
		if (ImGui::BeginMainMenuBar())
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
			ImGui::EndMainMenuBar();
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

		if(IsShowConsoleLog)
			ShowConsoleLogs(&IsShowConsoleLog);

		if(IsShowControlServer)
			p_controlServer->Draw(&IsShowControlServer);

#ifndef IMGUI_DISABLE_DEBUG_TOOLS
		if(IsShowDebug)
			ImGui::ShowDebugLogWindow(&IsShowDebug);
#endif
		g_pNotif->Draw();
		pUrls->Draw();
		if(IsShowSetting)
			gSetting->Draw(&IsShowSetting);
		/** * * * * * * * * * * * * * * **/

		ImGui::Render();
		const float clear_color_with_alpha[4] = {clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w};
	
		g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
		g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);

		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}

		g_pSwapChain->Present(1, 0);
	}

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

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if(ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
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