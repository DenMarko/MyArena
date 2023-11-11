#pragma once
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "CSetting.h"
#include "CDevice3D.h"

namespace SpaceUI
{
	class CImGui
	{
	public:
		CImGui();
		~CImGui();

		void Init(HWND hWnd, ID3D11Device* d3Device, ID3D11DeviceContext* d3DeviceContext);
		void Shutdown();

		void Begin();
		void End(const shared_ptr<Space3D::CDevice3D> &gDevice);

		void BeginDockSpace(const ImGuiViewport* viewport, HWND hWnd);
		void RenderWindowOuterBorders(ImGuiWindow* window);
		void EndDockSpace();

		bool IsMouseHovered(const ImVec2& MousePos, const ImVec2& pos_min, const ImVec2& pos_max);
		bool IsCloseButtomDown(const ImVec2& MousePos, const ImVec2& pos_min, const ImVec2& pos_max);
		bool IsMinimiseButtomDown(const ImVec2& MousePos, const ImVec2& pos_min, const ImVec2& pos_max);
		bool IsMaximiseButtomDown(const ImVec2& MousePos, const ImVec2& pos_min, const ImVec2& pos_max);

	private:
		ImVec4 clear_color;
	};
}
