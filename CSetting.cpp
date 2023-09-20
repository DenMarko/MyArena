#include "CSetting.h"
#include <ShlObj.h>
#include <io.h>
#include <direct.h>
#include <writer.h>
#include <stringbuffer.h>

Globals _glob_;
Globals *g_pGlob = &_glob_;

CSetting::CSetting()
{
	char localAppDataPath[MAX_PATH];
	if (SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppDataPath) == S_OK)
	{
		char fullPath[MAX_PATH];
		snprintf(fullPath, sizeof(fullPath), "%s\\MyArena", localAppDataPath);
		if (_access(fullPath, 0) == 0)
		{
			snprintf(fullPath, sizeof(fullPath), "%s\\MyArena\\setting.txt", localAppDataPath);
			FILE *f = nullptr;
			if (fopen_s(&f, fullPath, "r+") == 0)
			{
				fseek(f, 0, SEEK_END);
				size_t fSize = ftell(f);
				rewind(f);

				char* buf = new char[fSize + 1];
				fread(buf, 1, fSize, f);
				buf[fSize] = '\0';
				fclose(f);

				if (!doc.Parse(buf).HasParseError())
				{
					if (doc.HasMember("token"))
					{
						if (doc["token"].IsString())
						{
							_glob_.token.append(decrypt(string(doc["token"].GetString()), 3));
							_glob_.IsTocken = true;
						}
					}
					if(doc.HasMember("FontSize"))
					{
						if (doc["FontSize"].IsFloat())
						{
							_glob_.fFontSize = doc["FontSize"].GetFloat();
						}
					}
					if (doc.HasMember("Style"))
					{
						if (doc["Style"].IsInt())
						{
							_glob_.enumStyle = (doc["Style"].GetInt() == 0 ? Style_Dark : doc["Style"].GetInt() == 1 ? Style_Light : Style_Classic);
						}
					}
					if (doc.HasMember("IntervalConsole"))
					{
						if (doc["IntervalConsole"].IsFloat())
						{
							_glob_.fIntervalServerConsole = doc["IntervalConsole"].GetFloat();
						}
					}
					if (doc.HasMember("IntervalControl"))
					{
						if (doc["IntervalControl"].IsFloat())
						{
							_glob_.fIntervalControlServer = doc["IntervalControl"].GetFloat();
						}
					}
					if (doc.HasMember("Lang"))
					{
						if (doc["Lang"].IsInt())
						{
							_glob_.enumLang = (doc["Lang"].GetInt() == 0 ? LANG::UA : doc["Lang"].GetInt() == 1 ? LANG::RU : LANG::EN);
						}
					}
				}
				delete[] buf;
			} else {
				doc.SetObject();
			}
		}
		else
		{
			_mkdir(fullPath);
			doc.SetObject();
		}
	}
}

CSetting::~CSetting()
{
	char localAppDataPath[MAX_PATH];
	if (SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, localAppDataPath) == S_OK)
	{
		char fullPath[MAX_PATH];
		snprintf(fullPath, sizeof(fullPath), "%s\\MyArena", localAppDataPath);
		if (_access(fullPath, 0) != 0)
		{
			_mkdir(fullPath);
		}

		snprintf(fullPath, sizeof(fullPath), "%s\\MyArena\\setting.txt", localAppDataPath);
		FILE *f = nullptr;

		auto ret = fopen_s(&f, fullPath, "w+");
		if (ret == 0)
		{
			if (_glob_.IsWriteToken)
			{
				if (doc.HasMember("token"))
				{
					auto tocken = encrypt(_glob_.token.c_str(), 3);
					doc["token"].SetString(tocken.c_str(), tocken.length(), doc.GetAllocator());
				}
				else
				{
					rapidjson::Value vToken;
					auto tocken = encrypt(_glob_.token.c_str(), 3);
					vToken.SetString(tocken.c_str(), tocken.length(), doc.GetAllocator());
					doc.AddMember("token", vToken, doc.GetAllocator());
				}
			}
			else
			{
				if (!doc.HasMember("token"))
				{
					doc.AddMember("token", "", doc.GetAllocator());
				}
			}

			if (doc.HasMember("FontSize"))
			{
				doc["FontSize"].SetFloat(_glob_.fFontSize);
			}
			else
			{
				doc.AddMember("FontSize", _glob_.fFontSize, doc.GetAllocator());
			}

			if (doc.HasMember("Style"))
			{
				doc["Style"].SetInt(_glob_.enumStyle == Style_Dark ? 0 : _glob_.enumStyle == Style_Light ? 1 : 2);
			}
			else
			{
				doc.AddMember("Style", (_glob_.enumStyle == Style_Dark ? 0 : _glob_.enumStyle == Style_Light ? 1 : 2), doc.GetAllocator());
			}

			if (doc.HasMember("IntervalConsole"))
			{
				doc["IntervalConsole"].SetFloat(_glob_.fIntervalServerConsole);
			}
			else
			{
				doc.AddMember("IntervalConsole", _glob_.fIntervalServerConsole, doc.GetAllocator());
			}

			if (doc.HasMember("IntervalControl"))
			{
				doc["IntervalControl"].SetFloat(_glob_.fIntervalControlServer);
			}
			else
			{
				doc.AddMember("IntervalControl", _glob_.fIntervalControlServer, doc.GetAllocator());
			}

			if (doc.HasMember("Lang"))
			{
				doc["Lang"].SetInt(_glob_.enumLang == LANG::UA ? 0 : _glob_.enumLang == LANG::RU ? 1 : 2);
			}
			else
			{
				doc.AddMember("Lang", (_glob_.enumLang == LANG::UA ? 0 : _glob_.enumLang == LANG::RU ? 1 : 2), doc.GetAllocator());
			}

			rapidjson::StringBuffer buf;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
			doc.Accept(writer);
			fprintf_s(f, buf.GetString());

			fclose(f);
		}
	}
}

void CSetting::SetStile()
{
	switch (_glob_.enumStyle)
	{
	case Style_Dark:
		ImGui::StyleColorsDark();
		break;
	case Style_Classic:
		ImGui::StyleColorsClassic();
		break;
	case Style_Light:
		ImGui::StyleColorsLight();
		break;
	}

	auto &style = ImGui::GetStyle();

	if (_glob_.pIO->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	style.FrameRounding = 2;
	style.GrabRounding = 2;
}

void CSetting::Draw(bool *o_open)
{
	ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin(gLangManager->GetLang("Settings"), o_open))
	{
		ImGui::End();
		return;
	}

	const char *combStyle[3] = {gLangManager->GetLang("Dark theme"), gLangManager->GetLang("Bright theme"), gLangManager->GetLang("A classic theme")};
	const char *combPrevVal = combStyle[_glob_.enumStyle];
	if (ImGui::BeginCombo(gLangManager->GetLang("Color theme"), combPrevVal))
	{
		int n = 0;
		for (int i = 0; i < 3; i++)
		{
			const bool is_select = (_glob_.enumStyle == n);
			if (ImGui::Selectable(combStyle[i], is_select))
			{
				_glob_.enumStyle = (n == 0 ? Style_Dark : n == 1 ? Style_Light : Style_Classic);
				SetStile();
			}

			if (is_select)
			{
				ImGui::SetItemDefaultFocus();
			}
			n++;
		}
		ImGui::EndCombo();
	}

	if (ImGui::SliderFloat(gLangManager->GetLang("Font size"), &_glob_.fFontSize, 8.0f, 20.f, "%1.f", ImGuiSliderFlags_AlwaysClamp))
	{
		_glob_.pFont->Scale = _glob_.fFontSize / _glob_.pFont->FontSize;
	}

	ImGui::Spacing();
	ImGui::SeparatorText(gLangManager->GetLang("Change the data update time"));
	ImGui::Spacing();

	const char *combIntervalLog[5] = {u8"2 sec", u8"3 sec", u8"4 sec", u8"5 sec", u8"6 sec"};
	const char *combPrevValInterval = combIntervalLog[static_cast<int>(_glob_.fIntervalServerConsole) - 2];
	if(ImGui::BeginCombo(gLangManager->GetLang("Console"), combPrevValInterval, ImGuiComboFlags_NoPreview))
	{
		for (int i = 0; i < 5; i++)
		{
			const bool is_select = ((static_cast<int>(_glob_.fIntervalServerConsole) - 2) == i);
			if(ImGui::Selectable(combIntervalLog[i], is_select))
			{
				_glob_.fIntervalServerConsole = static_cast<float>(i + 2);
				_glob_.g_ServerConsole->SetNewInterval(_glob_.fIntervalServerConsole);
			}

			if (is_select)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	ImGui::SameLine();

	const char *combIntervalControl[4] = {u8"5 sec", u8"10 sec", u8"15 sec", u8"20 sec"};
	const char *combPrevInterControl = combIntervalControl[_glob_.fIntervalControlServer == 5.f ? 0 : _glob_.fIntervalControlServer == 10.f ? 1 : _glob_.fIntervalControlServer == 15.f ? 2 : 3];
	if (ImGui::BeginCombo(gLangManager->GetLang("Server information"), combPrevInterControl, ImGuiComboFlags_NoPreview))
	{
		for (int i = 0; i < 4; i++)
		{
			const bool isSelect = ((_glob_.fIntervalControlServer == 5.f ? 0 : _glob_.fIntervalControlServer == 10.f ? 1 : _glob_.fIntervalControlServer == 15.f ? 2 : 3) == i); 
			if (ImGui::Selectable(combIntervalControl[i], isSelect))
			{
				switch (i)
				{
					case 0:
						_glob_.fIntervalControlServer = 5.f;
						break;
					case 1:
						_glob_.fIntervalControlServer = 10.f;
						break;
					case 2:
						_glob_.fIntervalControlServer = 15.f;
						break;
					case 3:
						_glob_.fIntervalControlServer = 20.f;
						break;
				}

				_glob_.g_ControlServer->SetNewInterval(_glob_.fIntervalControlServer);
			}

			if (isSelect)
			{
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndCombo();
	}

	ImGui::Spacing();
	ImGui::SeparatorText(gLangManager->GetLang("Language"));
	ImGui::Spacing();

	const char *combLang[3] = {gLangManager->GetLang("Ukrainian"), gLangManager->GetLang("Russian"), gLangManager->GetLang("English")};
	const char *combPrevLang = combLang[(_glob_.enumLang == LANG::UA ? 0 : _glob_.enumLang == LANG::RU ? 1 : 2)];
	if(ImGui::BeginCombo(gLangManager->GetLang("Language"), combPrevLang))
	{
		for (int i = 0; i < 3; i++)
		{
			const bool is_select = ((_glob_.enumLang == LANG::UA ? 0 : _glob_.enumLang == LANG::RU ? 1 : 2) == i);
			if(ImGui::Selectable(combLang[i], is_select))
			{
				_glob_.enumLang = (i == 0 ? LANG::UA : i == 1 ? LANG::RU : LANG::EN);
			}

			if (is_select)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}


	ImGui::End();
	return;
}