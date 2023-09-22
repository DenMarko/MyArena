#include "CLangManager.h"
#include "CSetting.h"

CLangManager LangManager;
CLangManager *gLangManager = &LangManager;

map<const char*, const char*, CPrtCmp> LangUa = 
{
	{"Server management",				u8"��������� ��������"},
	{"Status: Disabled",				u8"������: ���������"},
	{"Status: Enabled",					u8"������: ���������"},
	{"Status: Reboot",					u8"������: ������������"},
	{"Map",								u8"����"},
	{"Players",							u8"������"},
	{"Server ID",						u8"ID �������"},
	{"Days until the end of the lease",	u8"��� �� ��������� ������"},
	{"Management",						u8"���������"},
	{"Start",							u8"�����"},
	{"Stop",							u8"����"},
	{"Restart",							u8"�������"},
	{"Go to the server",				u8"����� �� ������"},
	{"Change map",						u8"������ �����"},
	{"Player name",						u8"��'� ������"},
	{"Frags",							u8"�����"},
	{"Time in game",					u8"��� � ��"},
	{"The server is already enabled",	u8"������ ��� ���������"},
	{"The server reboots",				u8"������ ���������������"},
	{"The server is already down",		u8"������ ��� ���������"},
	{"The server is down",				u8"������ ���������"},
	{"The server is already rebooting",	u8"������ ��� ���������������"},

	{"Server console",					u8"������� �������"},
	{"Enter the command",				u8"������ �������"},

	{"File",							u8"����"},
	{"Settings",						u8"���������"},
	{"Exit",							u8"�����"},

	{"Color theme",						u8"��������� ����"},
	{"Dark theme",						u8"�����"},
	{"Bright theme",					u8"�����"},
	{"Classic theme",					u8"��������"},
	{"Font size",						u8"����� ������"},
	{"Change the data update time",		u8"������ ��� ��������� �����"},
	{"Console",							u8"�������"},
	{"Server information",				u8"���������� ��� ������"},
	{"Ukrainian",						u8"���������"},
	{"English",							u8"English"},
	{"Russian",							u8"������"},
	{"Language",						u8"����"},
	
	{"Enter your server token",			u8"������ ��� ����� �� �������"},
	{"Save token",						u8"�������� �����"},
	{"Not recommended for public computers!!!",		u8"�� ������������� ��� �������� ���������!!!"},
	{"Connection error, try again later!",			u8"������ �������, ��������� �����!"},
	{"Incorrect answer!",				u8"���������� �������!"},
	{"Correctly led token!",			u8"�������� ������� �����!"},
	{"Incorrect token entered!",		u8"����������� �������� �����!"},

	{"Version of the components used in the program:", u8"����� ����������,\r\n�� ���������������� � �������:"},
	{"About",							u8"��� ��������"},
	{"Help",							u8"�������"},
};
map<const char*, const char*, CPrtCmp> LangRu = 
{	
	{"Server management",				u8"���������� ��������"},
	{"Status: Disabled",				u8"������: ��������"},
	{"Status: Enabled",					u8"������: �������"},
	{"Status: Reboot",					u8"������: ������������"},
	{"Map",								u8"�����"},
	{"Players",							u8"������"},
	{"Server ID",						u8"ID �������"},
	{"Days until the end of the lease",	u8"���� �� ��������� ������"},
	{"Management",						u8"����������"},
	{"Start",							u8"�����"},
	{"Stop",							u8"����"},
	{"Restart",							u8"�������"},
	{"Go to the server",				u8"����� �� ������"},
	{"Change map",						u8"�������� �����"},
	{"Player name",						u8"���"},
	{"Frags",							u8"�����"},
	{"Time in game",					u8"����� � ����"},
	{"The server is already enabled",	u8"������ ��� �������"},
	{"The server reboots",				u8"������ ���������������"},
	{"The server is already down",		u8"������ ��� ��������"},
	{"The server is down",				u8"������ ��������"},
	{"The server is already rebooting",	u8"������ ��� ���������������"},

	{"Server console",					u8"������� �������"},
	{"Enter the command",				u8"������ �������"},

	{"File",							u8"����"},
	{"Settings",						u8"���������"},
	{"Exit",							u8"�����"},

	{"Color theme",						u8"�������� ����"},
	{"Dark theme",						u8"������"},
	{"Bright theme",					u8"�������"},
	{"Classic theme",					u8"������������"},
	{"Font size",						u8"������ ������"},
	{"Change the data update time",		u8"�������� ����� ���������� ������"},
	{"Console",							u8"�������"},
	{"Server information",				u8"���������� � �������"},
	{"Ukrainian",						u8"���������"},
	{"Russian",							u8"������"},
	{"English",							u8"English"},
	{"Language",						u8"����"},
	
	{"Enter your server token",			u8"������� ����� �� �������"},
	{"Save token",						u8"��������� �����"},
	{"Not recommended for public computers!!!",		u8"�� ������������� ��� ��������� �����������!!!"},
	{"Connection error, try again later!",			u8"������ ����������, ���������� �����!"},
	{"Incorrect answer!",				u8"������������ �����!"},
	{"Correctly led token!",			u8"��������� �������� �����!"},
	{"Incorrect token entered!",		u8"������ ������������ �����!"},

	{"Version of the components used in the program:", u8"������ �����������,\r\n������������ � ���������:"},
	{"About",							u8"� ��������"},
	{"Help",							u8"�������"},
};

const char *CLangManager::GetLang(const char *str) const
{
	switch (g_pGlob->enumLang)
	{
	case LANG::UA:
		return LangUa[str];
	case LANG::RU:
		return LangRu[str];
	case LANG::EN:
		return str;
	}
	return '\0';
}
