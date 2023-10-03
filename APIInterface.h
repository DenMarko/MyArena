#pragma once
#include <string>
#include <vector>

using namespace std;

enum STATUS {
	STATUS_ERROR = 0,
	STATUS_OK
};

struct GetConsole
{
	GetConsole() 
	{
		console_log = nullptr;
	}
	~GetConsole()
	{
		if (console_log)
		{
			delete[] console_log;
		}
	}

	STATUS						s;
	char*						console_log;
};

struct CmdResult
{
	string						msg;
	STATUS						status;
};

struct GetMapList
{
	vector<string>				maps;
	STATUS						status;
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
		data = new GetData();
		server_id = 0;
		server_dateblock = 0;
		server_daytoblock = 0;
		online = -1;
		status = STATUS_OK;
	}
	~GetStatus()
	{
		delete data;
	}

	GetData*					data;
	int							server_id;
	int							server_dateblock;
	int							server_daytoblock;
	int							online;
	string						JoinServer;
	string						MapImg;
	STATUS						status;
};

struct TokenResult
{
	TokenResult()
	{
		status = STATUS::STATUS_ERROR;
		SServer = nullptr;
	}

	~TokenResult()
	{
		if(SServer != nullptr)
		{
			delete SServer;
		}
	}

	string						msg;
	GetStatus					*SServer;
	STATUS						status;
};
