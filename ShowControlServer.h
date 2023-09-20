#pragma once
#include <d3d11.h>
#include <imgui.h>
#include "C_Time.h"
#include "C_CUrl.h"
#include <stdio.h>

class CControlServer : public ITimerEvent
{
public:
	CControlServer(ID3D11Device* pDevice, C_CUrl *p_Url);
	~CControlServer()
	{
		delete p_gStatus;
		delete p_gMaps;

		if (im_texture)
		{
			im_texture->Release();
			im_texture = nullptr;
		}

		if (im_texture_url)
		{
			im_texture_url->Release();
			im_texture_url = nullptr;
		}
	}

	void Draw(bool *IsOpen);

	virtual TimerResult OnTimer(void *pData);
	virtual void OnTimerEnd(void *pData)
	{}

private:
	bool LoadBuffer(char* filename, const size_t fSize, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height);

private:
	ID3D11ShaderResourceView* im_texture;
	int im_width;
	int im_height;

	ID3D11ShaderResourceView* im_texture_url;
	int im_width_url;
	int im_height_url;

	ID3D11Device	*g_pDevice;

	C_CUrl			*p_gUrl;
	GetStatus		*p_gStatus;
	GetStatus		*Status;
	GetMapList		*p_gMaps;
	std::string		prevMap;

	atomic<bool> IsStatusReady;
};