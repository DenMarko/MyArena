#include "CDevice3D.h"

namespace Space3D
{
	CDevice3D::CDevice3D() : g_pd3dDevice(nullptr), g_pd3dDeviceContext(nullptr), g_pSwapChain(nullptr), g_mainRenderTargetView(nullptr), g_SwapChainOccluded(false)
	{
	}

	CDevice3D::~CDevice3D()
	{
	}

	void CDevice3D::CreateDeviceD3D(HWND hWnd)
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
		{
			res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
		}

		if (res != S_OK)
		{
			throw Exception(TEXT_(__FILE__), __LINE__, L"Failed to create Device Direct 3D.");
		}

		CreateRenderTarget();
	}

	void CDevice3D::CleanupDeviceD3D()
	{
		CleanupRenderTarget();
		if (g_pSwapChain)
		{
			g_pSwapChain->Release();
			g_pSwapChain = nullptr;
		}

		if (g_pd3dDeviceContext)
		{
			g_pd3dDeviceContext->Release();
			g_pd3dDeviceContext = nullptr;
		}

		if (g_pd3dDevice)
		{
			g_pd3dDevice->Release();
			g_pd3dDevice = nullptr;
		}
	}

	void CDevice3D::CreateRenderTarget()
	{
		ID3D11Texture2D* pBackBuffer;
		g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
		g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
		pBackBuffer->Release();
	}

	void CDevice3D::CleanupRenderTarget()
	{
		if (g_mainRenderTargetView)
		{
			g_mainRenderTargetView->Release();
			g_mainRenderTargetView = nullptr;
		}
	}

	void CDevice3D::ResizeBuffer(SpaceWin::size_window &sizes)
	{
		CleanupRenderTarget();
		g_pSwapChain->ResizeBuffers(0, sizes.g_Width, sizes.g_Height, DXGI_FORMAT_UNKNOWN, 0);
		CreateRenderTarget();
	}

	bool CDevice3D::SwapChainOccluded()
	{
		if (g_SwapChainOccluded && (g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED))
		{
			return true;
		}
		g_SwapChainOccluded = false;
		return false;
	}

	void CDevice3D::VSync(VSYNCH synch)
	{
		HRESULT hr = 0;
		switch (synch)
		{
		case VSYNCH_DISABLE:
			hr = g_pSwapChain->Present(0, 0);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			break;
		case VSYNCH_ENABLE:
			hr = g_pSwapChain->Present(1, 0);
			break;
		}
		g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
	}

	void CDevice3D::ClearRender(const float* clear)
	{
		g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
		g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear);
	}
}