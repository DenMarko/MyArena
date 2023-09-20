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

struct CPrtCmp
{
	bool operator() (const char *str1, const char *str2) const
	{
		return strcmp(str1, str2) < 0;
	}
};

class CLangManager
{
public:
	CLangManager(){}
	~CLangManager(){}

	const char *GetLang(const char *) const;
};

extern CLangManager *gLangManager;