#pragma once
#include "imgui_impl_win32.h"
#include "CException.h"
#include "CSetting.h"
#include "resource.h"

namespace SpaceWin
{
	struct size_window
	{
		size_window() : g_Height(0), g_Width(0), IsEmpty(false){}
		size_window(const UINT width, const UINT height) : g_Height(height), g_Width(width), IsEmpty(true) {}
		size_window(const size_window& other) : g_Height(other.g_Height), g_Width(other.g_Width), IsEmpty(true) {}

		void reset()
		{
			g_Width = g_Height = 0;
			IsEmpty = false;
		}
		void SetValue(const UINT w, const UINT h)
		{
			g_Width = w;
			g_Height = h;
			IsEmpty = true;
		}
		bool empty() { return IsEmpty; }
		size_window& operator = (const size_window &val)
		{
			if (this != &val)
			{
				g_Height = val.g_Height;
				g_Width = val.g_Width;
				IsEmpty = val.IsEmpty;
			}
			return *this;
		}

		UINT g_Width;
		UINT g_Height;
		bool IsEmpty;
	};
	struct WinProcValues
	{
		bool m_TitleBar = false;
		bool IsReadiWindow = false;
	};

	class CWinWin
	{
		class Exception : public CBaseException
		{
		public:
			Exception(const char *file, int line, const char *note) : CBaseException(file, line, note)
			{}
			virtual std::string GetFullMessage() const override
			{
				return GetNote() + "\nAt: " + GetLocation();
			}
			virtual std::string GetExceptionType() const override
			{
				return "Windows Exception";
			}
		};

	public:
		CWinWin(HINSTANCE hInst);
		~CWinWin();

		bool CreateWind(int Width, int Height);
		void Destroy();

		void setTitle(bool t);
		bool GetResize(size_window &siz);
		void SystemCommand(WPARAM wParam);
		void Show(int);

		bool IsWindowActive();
		const HWND operator() () const
		{
			return hWind;
		}
	private:
		static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	private:
		HWND hWind;
		WNDCLASSEXW wc;
	};
}
