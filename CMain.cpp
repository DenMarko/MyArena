#include "CMain.h"

namespace SpaceMain
{
	_threat::_threat() : m_IsPendingExit(false)
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

	_threat::~_threat()
	{
		if (!m_IsPendingExit)
		{
			m_IsPendingExit = true;
			m_CondVar.notify_one();
			m_Thread->join();
		}
	}


	CMain::CMain(HINSTANCE hInst, int Width, int Height) : gSetting(nullptr), 
	p_controlServer(nullptr), Showlog(nullptr), th(nullptr), gListServer(nullptr), 
	pFileInfo(nullptr), gWin(nullptr), gImGui(nullptr), gDevice3D(nullptr)
	{
		done = false;
		IsShowSetting = false;
	#ifndef IMGUI_DISABLE_DEBUG_TOOLS
		IsShowDebug = true;
	#endif

		IsShowAbout = false;

		gSetting = make_shared<CSetting>();
		GetResourceInfo(hInst);

		gWin = make_shared<SpaceWin::CWinWin>(hInst);
		gWin->CreateWind(Width, Height);

		gDevice3D = make_shared<Space3D::CDevice3D>();
		gDevice3D->CreateDeviceD3D((*gWin)());

		gImGui = make_shared<SpaceUI::CImGui>();
		gSetting->SetStile();

		gImGui->Init((*gWin)(), gDevice3D->GetDevice(), gDevice3D->GetContext());
	
		p_controlServer = make_shared<CControlServer>(gDevice3D->GetDevice());
		Showlog = make_shared<CShowConsoleLog>();
		gListServer = make_shared<CListServer>();

		th = make_unique<_threat>();

		g_pGlob->g_ServerConsole = timer->CreateTimer(Showlog,			g_pGlob->fIntervalServerConsole, nullptr);
		g_pGlob->g_ControlServer = timer->CreateTimer(p_controlServer,	g_pGlob->fIntervalControlServer, nullptr);

		PushRenderUI(gSetting,			&IsShowSetting);
		PushRenderUI(Showlog);
		PushRenderUI(p_controlServer);
		PushRenderUI(gListServer);
		PushRenderUI(g_pNotif);
		PushRenderUI(pUrls);
	}

	CMain::~CMain()
	{
		for(auto UI_Render : m_RenderSteck)
			UI_Render->OnDetach();

		m_RenderSteck.clear();

		gImGui->Shutdown();
		gDevice3D->CleanupDeviceD3D();
	}

	int CMain::Loop()
	{
		MSG msg = { NULL };
		double dCurTime = 0;
		double iMemUsed = 0;
		while (!done)
		{
			if (!gWin->IsWindowActive())
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}

			while (PeekMessageW(&msg, nullptr, 0U, 0U, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessageW(&msg);

				if (msg.message == WM_QUIT)
				{
					done = true;
				}
			}

			if(done)
			{
				break;
			}

			if (gDevice3D->SwapChainOccluded())
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				continue;
			}

			{
				SpaceWin::size_window winSize;
				if (gWin->GetResize(winSize))
				{
					gDevice3D->ResizeBuffer(winSize);
				}
			}

			gImGui->Begin();

			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			gImGui->BeginDockSpace(viewport, (*gWin)());

			gWin->setTitle(gImGui->IsMouseHovered(g_pGlob->pIO->MousePos, 
				ImVec2(viewport->WorkPos.x + 120.f, viewport->WorkPos.y + 2.f), 
				ImVec2(viewport->WorkPos.x + (ImGui::GetContentRegionAvail().x - 60.f), viewport->WorkPos.y + 18.f)));


			if (gImGui->IsCloseButtomDown(g_pGlob->pIO->MousePos, 
				ImVec2(viewport->WorkPos.x + (ImGui::GetContentRegionAvail().x - 19.f), viewport->WorkPos.y + 2.f), 
				ImVec2(viewport->WorkPos.x + (ImGui::GetContentRegionAvail().x - 1.f), viewport->WorkPos.y + 18.f)))
			{
				gWin->SystemCommand(SC_CLOSE);
			}

			if (gImGui->IsMinimiseButtomDown(g_pGlob->pIO->MousePos, 
				ImVec2(viewport->WorkPos.x + (ImGui::GetContentRegionAvail().x - 59.f), viewport->WorkPos.y + 2.f), 
				ImVec2(viewport->WorkPos.x + (ImGui::GetContentRegionAvail().x - 41.f), viewport->WorkPos.y + 18.f)))
			{
				gWin->SystemCommand(SC_MINIMIZE);
			}

			if (gImGui->IsMaximiseButtomDown(g_pGlob->pIO->MousePos, 
				ImVec2(viewport->WorkPos.x + (ImGui::GetContentRegionAvail().x - 39.f), viewport->WorkPos.y + 2.f), 
				ImVec2(viewport->WorkPos.x + (ImGui::GetContentRegionAvail().x - 21.f), viewport->WorkPos.y + 18.f)))
			{
				if(IsZoomed((*gWin)()))
					gWin->SystemCommand(SC_RESTORE);
				else
					gWin->SystemCommand(SC_MAXIMIZE);
			}

			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu(gLangManager->GetLang("File")))
				{
					if (ImGui::MenuItem(gLangManager->GetLang("Server console"), NULL, g_pGlob->IsShowConsoleLog)) { g_pGlob->IsShowConsoleLog = g_pGlob->IsShowConsoleLog ? false : true; }
					if (ImGui::MenuItem(gLangManager->GetLang("Server management"), NULL, g_pGlob->IsShowControlServer)) { g_pGlob->IsShowControlServer = g_pGlob->IsShowControlServer ? false : true; }
					if (ImGui::MenuItem(gLangManager->GetLang("List of servers"), NULL, g_pGlob->IsShowListServer)) { g_pGlob->IsShowListServer = g_pGlob->IsShowListServer ? false : true; }
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
				ImVec2 vCenter = viewport->GetCenter();
				ImVec2 sizeWin = {350.f, 200.f};
				ImGui::SetNextWindowPos(vCenter, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
				ImGui::SetNextWindowSize(sizeWin, ImGuiCond_Always);

				if (ImGui::Begin(gLangManager->GetLang("About"), &IsShowAbout))
				{
					if ((dCurTime + 2.0) <= ImGui::GetTime())
					{
						mem::mMemory.mem_compact();
						dCurTime = ImGui::GetTime();
						iMemUsed = (static_cast<double>(mem::mMemory.mem_usage()) / 1024.0) / 1024.0;
					}

					ImGui::Text(gLangManager->GetLang("Version of the components used in the program:"));

					ImGui::Text("ImGui ver: %s", IMGUI_VERSION);
					ImGui::Text("CURL  ver: %s", LIBCURL_VERSION);
					ImGui::Text("RAPIDJSON: %s", RAPIDJSON_VERSION_STRING);
					ImGui::Text("Memory used: %.3f KB", iMemUsed);
					if (pFileInfo != nullptr)
					{
						ImGui::Text("Version program: %s", pVersionProg.c_str());
					}

					ImGui::SetCursorPos(ImVec2((sizeWin.x - 80.f) * 0.5f, (sizeWin.y * 1.6f) * 0.5f));
					if(ImGui::Button("Ok", ImVec2(80, 0)))
					{
						IsShowAbout = false;
					}
				}
				ImGui::End();
			}

	#ifndef IMGUI_DISABLE_DEBUG_TOOLS
			if(IsShowDebug)
				ImGui::ShowDebugLogWindow(&IsShowDebug);
	#endif
			for(auto UI_Render : m_RenderSteck)
				UI_Render->OnUIRender();

			gImGui->EndDockSpace();
			gImGui->End(gDevice3D);

			gDevice3D->VSync(Space3D::VSYNCH_DISABLE);
		}
		return (int)msg.wParam;
	}

	void CMain::GetResourceInfo(HINSTANCE hInst)
	{
		HRSRC hRes = FindResource(hInst, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION);
		if (hRes == nullptr)
		{
			return;
		}

		HGLOBAL hResGlob = LoadResource(hInst, hRes);
		if (hResGlob == nullptr)
		{
			return;
		}

		void* pData = LockResource(hResGlob);
		if (pData != nullptr)
		{
			DWORD resSize = SizeofResource(hInst, hRes);
			if (resSize > 0)
			{
				size_t len = 0;
				VerQueryValue(pData, "\\", (void**)&pFileInfo, &len);

				LPVOID pValue = 0;
				len = 0;
				if (VerQueryValueW(pData, L"\\StringFileInfo\\000904b0\\ProductVersion", &pValue, &len))
				{
					size_t iLen = len * sizeof(char*);

					char* pVer = new char[iLen+1];
					memset(pVer, 0, iLen);
					memcpy(pVer, pValue, iLen);

					for (size_t i = 0; i < iLen; i++)
					{
						if (pVer[i] != '\0')
						{
							pVersionProg += pVer[i];
						}
						if (pVer[i] == '\0' && pVer[i + 1] == '\0')
						{
							break;
						}
					}
					delete pVer;
				}
			}
		}
		FreeResource(hResGlob);
	}
}