#include "CImGui.h"

namespace SpaceUI
{
	CImGui::CImGui()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		g_pGlob->pIO = &ImGui::GetIO();
		g_pGlob->pIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		g_pGlob->pIO->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		g_pGlob->pIO->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		g_pGlob->pIO->ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;

		g_pGlob->pIO->ConfigViewportsNoTaskBarIcon = true;
		g_pGlob->pIO->ConfigDockingTransparentPayload = true;

		g_pGlob->pFont = g_pGlob->pIO->Fonts->AddFontFromFileTTF("C:/Windows/fonts/consola.ttf", g_pGlob->fFontSize, nullptr, g_pGlob->pIO->Fonts->GetGlyphRangesCyrillic());
		g_pGlob->pIO->Fonts->Build();
	}

	CImGui::~CImGui()
	{
	}

	void CImGui::Init(HWND hWnd, ID3D11Device* d3Device, ID3D11DeviceContext* d3DeviceContext)
	{
		ImGui_ImplWin32_Init(hWnd);
		ImGui_ImplDX11_Init(d3Device, d3DeviceContext);
	}

	void CImGui::Shutdown()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void CImGui::Begin()
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
	}

	void CImGui::End(const shared_ptr<Space3D::CDevice3D> &gDevice)
	{
		ImGui::Render();

		const float clear_color_with_alpha[4] = {clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w};
		gDevice->ClearRender(clear_color_with_alpha);

		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		if (g_pGlob->pIO->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void CImGui::BeginDockSpace(const ImGuiViewport* viewport, HWND hWnd)
	{
		ImGuiWindowFlags win_flags = ImGuiWindowFlags_NoDocking;

		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		win_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		win_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar;

		const bool isMaximized = IsZoomed(hWnd);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, isMaximized ? ImVec2(6.f, 6.f) : ImVec2(1.f, 1.f));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);

		ImGui::Begin("DockSpace Demo", nullptr, win_flags);

		ImGui::PopStyleVar(2);
		ImGui::PopStyleVar(2);

		ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(50, 50, 50, 255));

		if(!isMaximized)
		{
			RenderWindowOuterBorders(ImGui::GetCurrentWindow());
		}
		ImGui::PopStyleColor();

		if (g_pGlob->pIO->ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGui::DockSpace(ImGui::GetID("DirectXAppDockSpace"));
		}
	}

	void CImGui::RenderWindowOuterBorders(ImGuiWindow* window)
	{
		struct ImGuiResizeBorderDef
		{
			ImVec2 InnerDir;
			ImVec2 SegmentN1, SegmentN2;
			float  OuterAngle;
		};

		static const ImGuiResizeBorderDef resize_border_def[4] =
		{{ ImVec2(+1, 0), ImVec2(0, 1), ImVec2(0, 0), IM_PI * 1.00f },
		{  ImVec2(-1, 0), ImVec2(1, 0), ImVec2(1, 1), IM_PI * 0.00f },
		{  ImVec2(0, +1), ImVec2(0, 0), ImVec2(1, 0), IM_PI * 1.50f },
		{  ImVec2(0, -1), ImVec2(1, 1), ImVec2(0, 1), IM_PI * 0.50f }};

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
		float border_size = 1.0f;
		if (border_size > 0.0f)
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
			window->DrawList->PathStroke(ImGui::GetColorU32(ImGuiCol_SeparatorActive), 0, ImMax(2.0f, border_size));
		}
		if (g.Style.FrameBorderSize > 0 && !window->DockIsActive)
		{
			float y = window->Pos.y + window->TitleBarHeight() - 1;
			window->DrawList->AddLine(ImVec2(window->Pos.x + border_size, y), ImVec2(window->Pos.x + window->Size.x - border_size, y), ImGui::GetColorU32(ImGuiCol_Border), g.Style.FrameBorderSize);
		}
	}

	void CImGui::EndDockSpace()
	{
		ImGui::End();
	}

	bool CImGui::IsMouseHovered(const ImVec2& MousePos, const ImVec2& pos_min, const ImVec2& pos_max)
	{
		if (MousePos.x >= pos_min.x && MousePos.x <= pos_max.x && MousePos.y >= pos_min.y && MousePos.y <= pos_max.y)
		{
			return true;
		}
		return false;
	}

	bool CImGui::IsCloseButtomDown(const ImVec2& MousePos, const ImVec2& pos_min, const ImVec2& pos_max)
	{
		auto gfDraw = ImGui::GetForegroundDrawList();
		bool ret = false , hov = false;

		if (MousePos.x >= pos_min.x && MousePos.x <= pos_max.x && MousePos.y >= pos_min.y && MousePos.y <= pos_max.y)
		{
			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			{
				ret = true;
			}
			hov = true;
		}

		ImRect bb(pos_min, pos_max);
		ImVec2 center = bb.GetCenter();

		if(hov)
			gfDraw->AddCircleFilled(center, ImMax(2.f, (g_pGlob->pFont->FontSize * 1.2f) * 0.5f + 1.f), ImGui::GetColorU32(ret ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered));

		center.x = center.x - 0.5f;
		center.y = center.y - 0.5f;

		float cross_ex = ((g_pGlob->pFont->FontSize * 1.2f) * 0.5f * 0.7071f - 1.f);
		ImU32 cross_col = ImGui::GetColorU32(ImGuiCol_Text);

		gfDraw->AddLine(ImVec2(center.x + cross_ex, center.y + cross_ex), ImVec2(center.x + -cross_ex, center.y + -cross_ex), cross_col, 1.0f);
		gfDraw->AddLine(ImVec2(center.x + cross_ex, center.y + -cross_ex), ImVec2(center.x + -cross_ex, center.y + cross_ex), cross_col, 1.0f);

		return ret;
	}

	bool CImGui::IsMinimiseButtomDown(const ImVec2& MousePos, const ImVec2& pos_min, const ImVec2& pos_max)
	{
		auto gfDraw = ImGui::GetForegroundDrawList();
		bool hover = false, held = false;

		if (MousePos.x >= pos_min.x && MousePos.x <= pos_max.x && MousePos.y >= pos_min.y && MousePos.y <= pos_max.y)
		{
			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
				held = true;
			}
			hover = true;
		}

		ImRect bb(pos_min, pos_max);
		ImVec2 center = bb.GetCenter();

		if(hover)
			gfDraw->AddCircleFilled(center, ImMax(2.f, (g_pGlob->pFont->FontSize * 1.2f) * 0.5f + 1.f), ImGui::GetColorU32(held ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered));

		center.x = center.x - 0.5f;
		center.y = center.y - 0.5f;

		ImU32 cross_col = ImGui::GetColorU32(ImGuiCol_Text);
		float cross_ex = ((g_pGlob->pFont->FontSize * 1.2f) * 0.5f * 0.7071f - 1.f);

		gfDraw->AddLine(ImVec2(center.x + cross_ex, center.y + (cross_ex * 0.5f)), ImVec2(center.x + -cross_ex, center.y + (cross_ex * 0.5f)), cross_col, 1.0f);

		return held;
	}

	bool CImGui::IsMaximiseButtomDown(const ImVec2& MousePos, const ImVec2& pos_min, const ImVec2& pos_max)
	{
		auto gfDraw = ImGui::GetForegroundDrawList();
		bool hover = false, held = false;

		if (MousePos.x >= pos_min.x && MousePos.x <= pos_max.x && MousePos.y >= pos_min.y && MousePos.y <= pos_max.y)
		{
			if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
				held = true;
			}
			hover = true;
		}

		ImRect bb(pos_min, pos_max);
		ImVec2 center = bb.GetCenter();

		if(hover)
			gfDraw->AddCircleFilled(center, ImMax(2.f, (g_pGlob->pFont->FontSize * 1.2f) * 0.5f + 1.f), ImGui::GetColorU32(held ? ImGuiCol_ButtonActive : ImGuiCol_ButtonHovered));

		center.x = center.x - 0.5f;
		center.y = center.y - 0.5f;

		ImU32 cross_col = ImGui::GetColorU32(ImGuiCol_Text);
		float cross_ex = ((g_pGlob->pFont->FontSize * 1.2f) * 0.5f * 0.7071f - 1.f);

		gfDraw->AddLine(ImVec2(center.x + -cross_ex, center.y + -cross_ex), ImVec2(center.x + -cross_ex, center.y + cross_ex), cross_col, 1.0f);
		gfDraw->AddLine(ImVec2(center.x + -cross_ex, center.y + -cross_ex), ImVec2(center.x + cross_ex, center.y + -cross_ex), cross_col, 1.0f);
		gfDraw->AddLine(ImVec2(center.x + -cross_ex, center.y + cross_ex), ImVec2(center.x + cross_ex, center.y + cross_ex), cross_col, 1.0f);
		gfDraw->AddLine(ImVec2(center.x + cross_ex, center.y + cross_ex), ImVec2(center.x + cross_ex, center.y + -cross_ex), cross_col, 1.0f);

		return held;
	}
}