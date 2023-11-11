#include "C_CUrl.h"
#include <document.h>
#include <error/en.h>

shared_ptr<C_CUrl> pUrls(make_shared<C_CUrl>());

C_CUrl::C_CUrl() : IsOpen(true)
{
	memset(buffer, 0, sizeof(buffer));

	curl_global_init(CURL_GLOBAL_ALL);
	pUrl = curl_easy_init();
	if(pUrl == nullptr)
		throw Exception(__FILE__, __LINE__, "Failed to create valid curl handle.");
}

C_CUrl::~C_CUrl()
{
	curl_easy_cleanup(pUrl);
	curl_global_cleanup();
}

void *C_CUrl::GetData(COMMAND comand, const char *str)
{
	void *ret = nullptr;
	if (g_pGlob->IsTocken)
	{
		SetToken();

		if(token == nullptr)
			return ret;

		Utilite::CArray<char> data;
		std::string res_url = "https://www.myarena.ru/api.php";
		std::unique_lock<std::mutex> Lock(mtx);

		switch (comand)
		{
		case STOP:
		{
			res_url.append("?query=");
			res_url.append(curl_easy_escape(pUrl, "stop", 0));
			res_url.append("&token=");
			res_url.append(curl_easy_escape(pUrl, token->token.c_str(), 0));

			ret = new CmdResult;

			if (!Curl_Perform(res_url, data))
			{
				((CmdResult *)ret)->status = ERRORS;
				break;
			}

			data.push('\0');
			rapidjson::Document doc;
			rapidjson::ParseResult ok = doc.Parse(data.data());
			if (!ok)
			{
				IMGUI_DEBUG_LOG("JSON parse error: (%u)%s \n", ok.Offset(), rapidjson::GetParseError_En(ok.Code()));
				((CmdResult *)ret)->status = ERRORS;
				break;
			}

			if (doc["status"].IsString())
			{
				if (strcmp(doc["status"].GetString(), "OK") == 0)
				{
					CountBedTocken = 0;
					((CmdResult *)ret)->status = OK;
				}
				else if (strcmp(doc["status"].GetString(), "NO") == 0)
				{
					((CmdResult *)ret)->status = ERRORS;
					if (doc["message"].IsString())
					{
						if (strcmp(doc["message"].GetString(), "bad request") == 0)
						{
							IMGUI_DEBUG_LOG("bad request\n");
							CheckBedTocken();
						}
						else if (strcmp(doc["message"].GetString(), "bad token") == 0)
						{
							IMGUI_DEBUG_LOG("bad token\n");
							CheckBedTocken();
						}
					}
					break;
				}
				else
				{
					((CmdResult *)ret)->status = ERRORS;
					break;
				}

				if (doc["message"].IsString())
				{
					((CmdResult *)ret)->msg.append(doc["message"].GetString());
				}
			}
			break;
		}
		case START:
			{
				res_url.append("?query=");
				res_url.append(curl_easy_escape(pUrl, "start", 0));
				res_url.append("&token=");
				res_url.append(curl_easy_escape(pUrl, token->token.c_str(), 0));

				ret = new CmdResult;
				if (!Curl_Perform(res_url, data))
				{
					((CmdResult *)ret)->status = ERRORS;
					break;
				}

				data.push('\0');
				rapidjson::Document doc;
				rapidjson::ParseResult ok = doc.Parse(data.data());
				if (!ok)
				{
					IMGUI_DEBUG_LOG("JSON parse error: (%u)%s \n", ok.Offset(), rapidjson::GetParseError_En(ok.Code()));
					((CmdResult *)ret)->status = ERRORS;
					break;
				}

				if (doc["status"].IsString())
				{
					if (strcmp(doc["status"].GetString(), "OK") == 0)
					{
						CountBedTocken = 0;
						((CmdResult *)ret)->status = OK;
					}
					else if(strcmp(doc["status"].GetString(), "NO") == 0)
					{
						((CmdResult *)ret)->status = ERRORS;
						if (doc["message"].IsString())
						{
							if (strcmp(doc["message"].GetString(), "bad request") == 0)
							{
								IMGUI_DEBUG_LOG("bad request\n");
								CheckBedTocken();
							}
							else if (strcmp(doc["message"].GetString(), "bad token") == 0)
							{
								IMGUI_DEBUG_LOG("bad token\n");
								CheckBedTocken();
							}
						}
						break;
					}
				}
				else
				{
					((CmdResult *)ret)->status = ERRORS;
					break;
				}

				if(doc["message"].IsString())
					((CmdResult *)ret)->msg.append(doc["message"].GetString());
			}
			break;
		case STATUS:
			{
				res_url.append("?query=");
				res_url.append(curl_easy_escape(pUrl, "status", 0));
				res_url.append("&token=");
				res_url.append(curl_easy_escape(pUrl, token->token.c_str(), 0));
				res_url.append("&ver=2");

				if (!Curl_Perform(res_url, data))
				{
					break;
				}

				data.push('\0');
				ret = GetParseStatus(data);
			}
			break;
		case RESTART:
			{
				res_url.append("?query=");
				res_url.append(curl_easy_escape(pUrl, "restart", 0));
				res_url.append("&token=");
				res_url.append(curl_easy_escape(pUrl, token->token.c_str(), 0));

				ret = new CmdResult;

				if (!Curl_Perform(res_url, data))
				{
					((CmdResult *)ret)->status = ERRORS;
					break;
				}

				data.push('\0');
				rapidjson::Document doc;
				rapidjson::ParseResult ok = doc.Parse(data.data());
				if (!ok)
				{
					IMGUI_DEBUG_LOG("JSON parse error: (%u)%s \n", ok.Offset(), rapidjson::GetParseError_En(ok.Code()));
					((CmdResult *)ret)->status = ERRORS;
					break;
				}

				if (doc["status"].IsString())
				{
					if (strcmp(doc["status"].GetString(), "OK") == 0)
					{
						CountBedTocken = 0;
						((CmdResult *)ret)->status = OK;
					}
					else if(strcmp(doc["status"].GetString(), "NO") == 0)
					{
						((CmdResult *)ret)->status = ERRORS;
						if (doc["message"].IsString())
						{
							if (strcmp(doc["message"].GetString(), "bad request") == 0)
							{
								IMGUI_DEBUG_LOG("bad request\n");
								CheckBedTocken();
							}
							else if (strcmp(doc["message"].GetString(), "bad token") == 0)
							{
								IMGUI_DEBUG_LOG("bad token\n");
								CheckBedTocken();
							}
						}
						break;
					}
				}
				else
				{
					((CmdResult *)ret)->status = ERRORS;
					break;
				}

				if(doc["message"].IsString())
					((CmdResult *)ret)->msg.append(doc["message"].GetString());
			}
			break;
		case GET_MAPS:
			{
				res_url.append("?query=");
				res_url.append(curl_easy_escape(pUrl, "getmaps", 0));
				res_url.append("&token=");
				res_url.append(curl_easy_escape(pUrl, token->token.c_str(), 0));

				if (!Curl_Perform(res_url, data))
				{
					break;
				}

				ret = new GetMapList();

				data.push('\0');
				rapidjson::Document doc;
				rapidjson::ParseResult ok = doc.Parse(data.data());
				if (!ok)
				{
					IMGUI_DEBUG_LOG("JSON parse error: (%u)%s \n", ok.Offset(), rapidjson::GetParseError_En(ok.Code()));
					((GetMapList *)ret)->status = ERRORS;
					break;
				}

				if (doc["status"].IsString())
				{
					if (strcmp(doc["status"].GetString(), "OK") == 0)
					{
						CountBedTocken = 0;
						if (doc.HasMember("maps") && doc["maps"].IsArray())
						{
							auto &mapArray = doc["maps"];
							for (rapidjson::SizeType i = 0; i < mapArray.Size(); ++i)
							{
								if (mapArray[i].IsString())
								{
									((GetMapList *)ret)->maps.emplace_back(mapArray[i].GetString());
								}
							}
							((GetMapList *)ret)->status = OK;
						}
					}
					else if(strcmp(doc["status"].GetString(), "NO") == 0)
					{
						((CmdResult *)ret)->status = ERRORS;
						if (doc["message"].IsString())
						{
							if (strcmp(doc["message"].GetString(), "bad request") == 0)
							{
								IMGUI_DEBUG_LOG("bad request\n");
								CheckBedTocken();
							}
							else if (strcmp(doc["message"].GetString(), "bad token") == 0)
							{
								IMGUI_DEBUG_LOG("bad token\n");
								CheckBedTocken();
							}
						}
						break;
					}
				}
				else
				{
					((CmdResult *)ret)->status = ERRORS;
					break;
				}
			}

			break;
		case GET_CONSOLE:
			{
				res_url.append("?query=");
				res_url.append(curl_easy_escape(pUrl, "getconsole", 0));
				res_url.append("&token=");
				res_url.append(curl_easy_escape(pUrl, token->token.c_str(), 0));

				if (!Curl_Perform(res_url, data))
				{
					break;
				}

				data.push('\0');
				ret = GetParseConsole(data);
				break;
			}
		case CHANGE_LEVEL:
			{
				res_url.append("?query=");
				res_url.append(curl_easy_escape(pUrl, "changelevel", 0));
				res_url.append("&map=");
				res_url.append(curl_easy_escape(pUrl, str, 0));
				res_url.append("&token=");
				res_url.append(curl_easy_escape(pUrl, token->token.c_str(), 0));

				ret = new CmdResult;

				if (!Curl_Perform(res_url, data))
				{
					((CmdResult *)ret)->status = ERRORS;
					break;
				}

				data.push('\0');
				rapidjson::Document doc;
				rapidjson::ParseResult ok = doc.Parse(data.data());
				if (!ok)
				{
					IMGUI_DEBUG_LOG("JSON parse error: (%u)%s \n", ok.Offset(), rapidjson::GetParseError_En(ok.Code()));
					((CmdResult *)ret)->status = ERRORS;
					break;
				}

				if (doc["status"].IsString())
				{
					if (strcmp(doc["status"].GetString(), "OK") == 0)
					{
						CountBedTocken = 0;
						((CmdResult *)ret)->status = OK;
					}
					else if(strcmp(doc["status"].GetString(), "NO") == 0)
					{
						((CmdResult *)ret)->status = ERRORS;
						if (doc["message"].IsString())
						{
							if (strcmp(doc["message"].GetString(), "bad request") == 0)
							{
								IMGUI_DEBUG_LOG("bad request\n");
								CheckBedTocken();
							}
							else if (strcmp(doc["message"].GetString(), "bad token") == 0)
							{
								IMGUI_DEBUG_LOG("bad token\n");
								CheckBedTocken();
							}
						}
						break;
					}
				}
				else
				{
					((CmdResult *)ret)->status = ERRORS;
					break;
				}

				if(doc["message"].IsString())
					((CmdResult *)ret)->msg.append(doc["message"].GetString());
			}
			break;
		case GET_RESOURCES:
			break;
		case CONSOLE_CMD:
			{
				res_url.append("?query=");
				res_url.append(curl_easy_escape(pUrl, "consolecmd", 0));
				res_url.append("&cmd=");
				res_url.append(curl_easy_escape(pUrl, str, 0));
				res_url.append("&token=");
				res_url.append(curl_easy_escape(pUrl, token->token.c_str(), 0));

				ret = new CmdResult;

				if (!Curl_Perform(res_url, data))
				{
					((CmdResult *)ret)->status = ERRORS;
					break;
				}

				data.push('\0');
				rapidjson::Document doc;
				rapidjson::ParseResult ok = doc.Parse(data.data());
				if (!ok)
				{
					IMGUI_DEBUG_LOG("JSON parse error: (%u)%s \n", ok.Offset(), rapidjson::GetParseError_En(ok.Code()));
					((CmdResult *)ret)->status = ERRORS;
					break;
				}

				if (doc["status"].IsString())
				{
					if (strcmp(doc["status"].GetString(), "OK") == 0)
					{
						CountBedTocken = 0;
						((CmdResult *)ret)->status = OK;
					}
					else if(strcmp(doc["status"].GetString(), "NO") == 0)
					{
						((CmdResult *)ret)->status = ERRORS;
						if (doc["message"].IsString())
						{
							if (strcmp(doc["message"].GetString(), "bad request") == 0)
							{
								IMGUI_DEBUG_LOG("bad request\n");
								CheckBedTocken();
							}
							else if (strcmp(doc["message"].GetString(), "bad token") == 0)
							{
								IMGUI_DEBUG_LOG("bad token\n");
								CheckBedTocken();
							}
						}
						break;
					}
				}
				else
				{
					((CmdResult *)ret)->status = ERRORS;
					break;
				}

				if(doc["message"].IsString())
					((CmdResult *)ret)->msg.append(doc["message"].GetString());
			}
			break;
		}
	}

	return ret;
}

void *C_CUrl::StatusToken(const char *cToken)
{
	Utilite::CArray<char> mData;
	std::string res_url = "https://www.myarena.ru/api.php";
	std::unique_lock<std::mutex> Lock(mtx);

	res_url.append("?query=");
	res_url.append(curl_easy_escape(pUrl, "status", 0));
	res_url.append("&token=");
	res_url.append(curl_easy_escape(pUrl, cToken, 0));
	res_url.append("&ver=2");

	TokenResult *res = new TokenResult();

	if (!Curl_Perform(res_url, mData))
	{
		res->status = ERRORS;
		res->msg.append(gLangManager->GetLang("Connection error, try again later!"));
		return res;
	}
	mData.push('\0');
	GetStatus *stats = reinterpret_cast<GetStatus *>(this->GetParseStatus(mData));

	if (stats->status == OK)
	{
		res->status = OK;
		res->SServer = stats;
		res->msg.append(gLangManager->GetLang("Correctly led token!"));
	}
	else if(stats->status == ERRORS)
	{
		res->status = ERRORS;
		res->SServer = stats;
		res->msg.append(gLangManager->GetLang("Incorrect token entered!"));
	}

	return res;
}

void *C_CUrl::GetParseConsole(Utilite::CArray<char> &mData)
{
	static std::string oldstr;
	GetConsole *con = new GetConsole;

	rapidjson::Document doc;
	rapidjson::ParseResult ok = doc.Parse(mData.data());
	if (!ok)
	{
		IMGUI_DEBUG_LOG("JSON parse error: (%u)%s \n", ok.Offset(), rapidjson::GetParseError_En(ok.Code()));
		con->s = ERRORS;
		return con;
	}

	if (doc["status"].IsString())
	{
		if (strcmp(doc["status"].GetString(), "OK") == 0)
		{
			CountBedTocken = 0;
			rapidjson::Value& con_log = doc["console_log"];
			if (con_log.IsString())
			{
				std::string newstr = (char *)con_log.GetString();
				if (oldstr.size() > 0)
				{
					size_t pos = findSubstring(oldstr, newstr);
					if (pos > 0 && pos < newstr.size())
					{
						CopiData(&con->console_log, newstr.substr(pos).c_str());
						con->s = OK;

						oldstr.clear();
						oldstr = newstr;
						return con;
					}
					else if (pos == newstr.size())
					{
						con->s = OK;
						return con;
					}
				}

				oldstr = newstr;
				CopiData(&con->console_log, newstr.c_str());
				con->s = OK;

				return con;
			}
		}
		else if(strcmp(doc["status"].GetString(), "NO") == 0)
		{
			con->s = ERRORS;
			if (doc["message"].IsString())
			{
				if (strcmp(doc["message"].GetString(), "bad request") == 0)
				{
					IMGUI_DEBUG_LOG("bad request\n");
					CheckBedTocken();
				}
				else if (strcmp(doc["message"].GetString(), "bad token") == 0)
				{
					IMGUI_DEBUG_LOG("bad token\n");
					CheckBedTocken();
				}
			}
			return con;
		}
	}

	con->s = ERRORS;
	return con;
}

void *C_CUrl::GetParseStatus(Utilite::CArray<char> &mData)
{
	GetStatus *res = new GetStatus();

	rapidjson::Document doc;
	rapidjson::ParseResult ok = doc.Parse(mData.data());
	if (!ok)
	{
		IMGUI_DEBUG_LOG("JSON parse error: (%u)%s \n", ok.Offset(), rapidjson::GetParseError_En(ok.Code()));
		res->status = ERRORS;
		return res;
	}

	if (doc["status"].IsString())
	{
		CountBedTocken = 0;
		if (strcmp(doc["status"].GetString(), "OK") == 0)
		{
			res->status = OK;

			if (doc["online"].IsInt())				{ res->online = doc["online"].GetInt();										}
			if (doc["server_id"].IsString())		{ res->server_id = atoi(doc["server_id"].GetString());						}
			if (doc["server_daystoblock"].IsInt())	{ res->server_daytoblock = doc["server_daystoblock"].GetInt();				}
			if (doc["server_address"].IsString())	{ res->data->addres.append(doc["server_address"].GetString());				}
			if (doc["map_img"].IsString())			{ res->MapImg = "https://"; res->MapImg.append(doc["map_img"].GetString());	}
			if (doc["server_dateblock"].IsString()) { res->server_dateblock = atoll(doc["server_dateblock"].GetString());		}

			if(res->online != 1)
			{
				return res;
			}

			auto& data = doc["data"];
			if (data.IsObject())
			{
				if (data["hostname"].IsString())	{ res->data->hostname.append(data["hostname"].GetString());	}
				if (data["map"].IsString())			{ res->data->mapname.append(data["map"].GetString());		}
				if (data["max_players"].IsInt())	{ res->data->maxPlayers = data["max_players"].GetInt();		}
				if (data["num_players"].IsInt())	{ res->data->numPlayers = data["num_players"].GetInt();		}
				if (data["steamappid"].IsInt())		{ res->data->steamappid = data["steamappid"].GetInt();		}
				if (data["version"].IsString())		{ res->data->version.append(data["version"].GetString());	}
				if (data["gq_joinlink"].IsString())	{ res->JoinServer.append(data["gq_joinlink"].GetString());	}

				auto &_player = data["players"];
				if (_player.IsArray())
				{
					for (rapidjson::SizeType i = 0; i < _player.Size(); i++)
					{
						auto &player = _player[i];
						if(player.IsObject())
						{
							GetPlayers newPlayer;
							if(player["name"].IsString())	{ newPlayer.name.append(player["name"].GetString()); } else { newPlayer.name.append("null"); }
							if(player["score"].IsInt())		{ newPlayer.score = player["score"].GetInt(); }
							if(player["time"].IsFloat())	{ formatTime(&newPlayer.time_to_game, player["time"].GetFloat()); }

							res->data->players.push_back(newPlayer);
						}
					}
				}
			}

			return res;
		}
		else if(strcmp(doc["status"].GetString(), "NO") == 0)
		{
			res->status = ERRORS;
			if (doc["message"].IsString())
			{
				if (strcmp(doc["message"].GetString(), "bad request") == 0)
				{
					IMGUI_DEBUG_LOG("bad request\n");
					CheckBedTocken();
				}
				else if (strcmp(doc["message"].GetString(), "bad token") == 0)
				{
					IMGUI_DEBUG_LOG("bad token\n");
					CheckBedTocken();
				}
			}
			return res;
		}
	}

	res->status = ERRORS;
	return res;
}

class TimeCallBack : public ITimerEvent
{
public:
	virtual TimerResult OnTimer(void *pData) override
	{
		reinterpret_cast<atomic<bool>*>(pData)->store(true);
		return TimerResult::Time_Stop;
	}
	virtual void OnTimerEnd(void *pData) override
	{}
};

void C_CUrl::OnUIRender()
{
	CheckToken();
	if (g_pGlob->IsTocken)
	{
		return;
	}
	static bool bFocus = false;
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Token", NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(gLangManager->GetLang("Enter your server token"));
		ImGui::Separator();

		if (bFocus) {
			ImGui::SetKeyboardFocusHere();
			bFocus = false;
		}

		ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_CallbackCompletion;
		if (ImGui::InputText("Input", buffer, IM_ARRAYSIZE(buffer), input_text_flags, &TEditCallbackStub, (void*)this))
		{
			char *s = buffer;
			if (s[0])
			{
				TokenResult *res =  reinterpret_cast<TokenResult *>(StatusToken(s));
				if (res->status == OK)
				{
					for (auto& sTok : g_pGlob->token)
					{
						if(sTok->IsActive)
							sTok->IsActive = false;
					}

					g_pGlob->token.emplace_back(make_shared<STokenList>(s, true));
					g_pGlob->token.back()->NServer = res->SServer->data->hostname;
					g_pGlob->IsWriteAtiveToken = true;
					g_pNotif->Notificatio(res->msg.c_str());

					ImGui::CloseCurrentPopup();
					g_pGlob->IsTocken = true;
					IsOpen = true;
				}
				else if (res->status == ERRORS)
				{
					ImGui::CloseCurrentPopup();
					g_pNotif->Notificatio(res->msg.c_str());
					timer->CreateTimer(make_shared<TimeCallBack>(), 3.2f, &IsOpen);
				}
				delete res;
			}
		}

		bFocus = ImGui::Checkbox(gLangManager->GetLang("Save token"), &g_pGlob->IsWriteToken);
		ImGui::SameLine();

		g_pNotif->HelpMarcer(gLangManager->GetLang("Not recommended for public computers!!!"));

		ImGui::SetCursorPos(ImVec2((ImGui::GetWindowSize().x * 0.5f) * 0.5f, ImGui::GetCursorPos().y));
		if (ImGui::Button(gLangManager->GetLang("Exit"), ImVec2(ImGui::GetWindowSize().x * 0.5f, 0.f)))
		{
			ImGui::CloseCurrentPopup();
			g_pGlob->IsTocken = true;
			IsOpen = true;
		}

		ImGui::EndPopup();
	}
}

void C_CUrl::LoadImageMap(std::string url_img, Utilite::CArray<char> *MapImag)
{
	CURLcode cod;			

	curl_easy_setopt(pUrl, CURLOPT_URL, url_img.c_str());
	curl_easy_setopt(pUrl, CURLOPT_WRITEFUNCTION, WriteData);
	curl_easy_setopt(pUrl, CURLOPT_WRITEDATA, MapImag);

	cod = curl_easy_perform(pUrl);
	curl_easy_reset(pUrl);

	if (cod != CURLE_OK)
	{
		MapImag->clear();
	}

	return;
}

void C_CUrl::CheckToken()
{
	if(IsOpen && !g_pGlob->IsTocken)
	{
		ImGui::OpenPopup("Token");
		IsOpen = false;
	}
}