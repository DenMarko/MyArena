#pragma once
#include "imgui.h"
#include "C_Time.h"
#include <assert.h>
#include "document.h"
#include "CLangManager.h"

using namespace std;

class CUIRender
{
public:
	virtual ~CUIRender() = default;

	virtual void OnAttach(bool *IsOpen) {}
	virtual void OnDetach() {}

	virtual void OnUIRender() {}
};

enum EnumStyle
{
	Style_Dark = 0,
	Style_Light,
	Style_Classic
};

struct STokenList
{
	STokenList(const char* Token, bool isActive) : token(Token), IsActive(isActive), IsSelect(false)
	{}

	string token;
	bool IsActive;
	string NServer;
	bool IsSelect;
};

struct Globals
{
	Globals() :	pIO(nullptr),
				pFont(nullptr),
				fFontSize(12),
				IsTocken(false),
				IsWriteToken(false),
				IsWriteAtiveToken(false),
				enumStyle(Style_Classic),
				g_ControlServer(nullptr),
				g_ServerConsole(nullptr),
				fIntervalControlServer(10.f),
				fIntervalServerConsole(3.f),
				IsShowConsoleLog(true),
				IsShowControlServer(true),
				IsShowListServer(true),
				enumLang(LANG::EN)
	{}
	~Globals(){}

	ImGuiIO *pIO;
	ImFont *pFont;

	float fFontSize;

	EnumStyle enumStyle;
	LANG enumLang;

	vector<shared_ptr<STokenList>> token;
	atomic<bool> IsTocken;
	bool IsWriteToken;
	bool IsWriteAtiveToken;

	ITimer *g_ControlServer;
	ITimer *g_ServerConsole;
	float fIntervalControlServer;
	float fIntervalServerConsole;

	bool IsShowConsoleLog;
	bool IsShowControlServer;
	bool IsShowListServer;
};

class CSetting : public CUIRender
{
public:
	CSetting();
	~CSetting();

	void SetStile();

	virtual void OnAttach(bool *IsOpen) override;
	virtual void OnDetach() override;

	virtual void OnUIRender() override;

private:
	string encrypt(string message, int shift)
	{
		string encrypted = "";
		for (char c : message)
		{
			if (isalpha(c))
			{
				char base = isupper(c) ? 'A' : 'a';
				encrypted += static_cast<char>((c - base + shift) % 26 + base);
			}
			else
			{
				encrypted += c;
			}
		}
		return encrypted;
	}
	string decrypt(string encrypted, int shift)
	{
		return encrypt(encrypted, 26 - shift);
	}
private:
	rapidjson::Document doc;
	bool *IsOpen;
};

extern Globals *g_pGlob;