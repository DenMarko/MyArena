#pragma once
#include <map>
#include <string>
#include <vector>

using namespace std;

enum LANG
{
	UA = 0,
	RU,
	EN
};

class CLangManager
{
public:
	CLangManager(){}
	~CLangManager(){}

	const char *GetLang(const char *) const;
};

extern unique_ptr<CLangManager> gLangManager;