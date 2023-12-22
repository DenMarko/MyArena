#pragma once
#include <string>
#include <vector>
#include "CMemory.h"

using namespace std;

enum STATS {
	NONE = -1,
	ERRORS,
	OK
};

struct GetConsole
{
	GetConsole() 
	{
		console_log = nullptr;
		s = NONE;
	}
	~GetConsole()
	{
		if (console_log)
		{
			m_delete(console_log);
		}
	}

	STATS						s;
	char*						console_log;
};

struct CmdResult
{
	CmdResult()
	{
		status = NONE;
	}
	~CmdResult() {}

	string						msg;
	STATS						status;
};

struct GetMapList
{
	GetMapList()
	{
		status = NONE;
	}
	~GetMapList() {}

	vector<string>				maps;
	STATS						status;
};

struct GetPlayers
{
	string						name;
	int							score;
	string						time_to_game;
};

struct GetData
{
	GetData()
	{
		maxPlayers = 0;
		numPlayers = 0;
		steamappid = 0;
	}
	~GetData()
	{
		//for(auto p : players)
		//	delete p;
	}
	
	string						addres;
	string						hostname;
	string						mapname;
	int							maxPlayers;
	int							numPlayers;
	vector<GetPlayers>			players;
	int							steamappid;
	string						version;
};

struct GetStatus 
{
	GetStatus()
	{
		data = m_new<GetData>();
		server_id = 0;
		server_dateblock = 0;
		server_daytoblock = 0;
		online = -1;
		status = NONE;
	}
	~GetStatus()
	{
		m_delete(data);
	}

	struct GetData*				data;
	int							server_id;
	long long					server_dateblock;
	int							server_daytoblock;
	int							online;
	string						JoinServer;
	string						MapImg;
	STATS						status;
};

struct TokenResult
{
	TokenResult()
	{
		status = NONE;
		SServer = nullptr;
	}

	~TokenResult()
	{
		if(SServer)
		{
			m_delete(SServer);
		}
	}

	string						msg;
	GetStatus					*SServer;
	STATS						status;
};
