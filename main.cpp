#include "CMain.h"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR pCommandLine, int nCommandShow)
{
	try
	{
		SpaceMain::CMain app(hInst, 1280, 800);
		return app.Loop();
	}
	catch( const CBaseException& e )
	{
		const std::string eMsg = e.GetFullMessage() + "\n\nException caught at Windows message loop.";

		MessageBox(nullptr, eMsg.c_str(), e.GetExceptionType().c_str(), MB_ICONERROR );
	}
	catch (const std::exception& e)
	{
		std::string whatStr(e.what());
		whatStr.append("\n\nException caught at Windows message loop.");
		MessageBox(nullptr, whatStr.c_str(),  "Unhandled STL Exception", MB_ICONERROR );
	}
	catch( ... )
	{
		MessageBox(nullptr, "\n\nException caught at Windows message loop.", "Unhandled Non-STL Exception", MB_ICONERROR );
	}

	return 0;
}

