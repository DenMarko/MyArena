#pragma once
#include <d3d11.h>
#include "CWinWin.h"

namespace Space3D
{
	enum VSYNCH
	{
		VSYNCH_DISABLE = 0,
		VSYNCH_ENABLE
	};

	class CDevice3D
	{
	public:
		CDevice3D();
		~CDevice3D();

		bool CreateDeviceD3D(HWND hWnd);
		void CleanupDeviceD3D();
		void CreateRenderTarget();
		void CleanupRenderTarget();
		void ResizeBuffer(SpaceWin::size_window &sizes);
		void VSync(VSYNCH synch);
		void ClearRender(const float *clear);

		ID3D11Device *GetDevice() { return g_pd3dDevice; }
		ID3D11DeviceContext *GetContext() { return g_pd3dDeviceContext; }

	private:
		ID3D11Device*            g_pd3dDevice;
		ID3D11DeviceContext*     g_pd3dDeviceContext;
		IDXGISwapChain*          g_pSwapChain;
		ID3D11RenderTargetView*  g_mainRenderTargetView;
	
	};
}
