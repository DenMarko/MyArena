#pragma once
#include "curl/include/curl.h"
#include "APIInterface.h"
#include "C_Time.h"
#include "imgui.h"
#include "CSetting.h"
#include "CNotification.h"

enum COMAND {
	COMMAND_STOP = 0,
	COMMAND_START,
	COMMAND_STATUS,
	COMMAND_RESTART,
	COMMAND_GET_MAPS,
	COMMAND_GET_CONSOLE,
	COMMAND_CHANGE_LEVEL,
	COMMAND_GET_RESOURCES,
	COMMAND_CONSOLE_CMD
};

class C_CUrl
{
public:
	C_CUrl();
	~C_CUrl();

	void *GetData(COMAND command, const char *cmd = nullptr);
	void Draw();
	void LoadImageMap(std::string url_img, vector<char> *MapImag);

private:
	void *SetToken(const char* cToken);
	void CheckToken();
	void *GetParseConsole(vector<char> &data);
	void *GetParseStatus(vector<char> &data);

	void CheckBedTocken()
	{
		CountBedTocken++;
		if (CountBedTocken > 5)
		{
			g_pGlob->token.clear();
			g_pGlob->IsTocken = false;
			IsOpen = true;
			CountBedTocken = 0;
		}
	}

	static size_t WriteData(char *str, size_t size, size_t nmemb, vector<char> *data)
	{
		size_t new_size = size*nmemb;
		size_t old_size = data->size();

		data->resize(new_size + old_size);
		memcpy(data->data() + (old_size == 0 ? old_size : (old_size - 1)), str, new_size);
		data->push_back('\0');

		return new_size;
	}

	unsigned int findSubstring(const std::string& str1, const std::string& str2)
	{
		auto iter1 = str1.begin();
		auto iter2 = str2.begin();
		unsigned int j = 0;

		while (iter1 != str1.end() && iter2 != str2.end())
		{
			if (*iter1 == *iter2)
			{
				iter2++;
			}
			else
			{
				j = distance(str2.begin(), iter2);
				if (j < 300)
				{
					iter1 -= j;
					iter2 = str2.begin();
				}
			}
			iter1++;
		}

		return distance(str2.begin(), iter2);
	}

	void CopiData(char **destStr, const char *sourceStr)
	{
		const size_t len = strlen(sourceStr) + 1;
		*destStr = new char[len];
		strcpy_s(*destStr, len, sourceStr);
	}

	void formatTime(std::string *buf, double time)
	{
		int hours = (int)(floor(time / 3600.0));
		int minutes = (int)(floor((time - hours * 3600.0) / 60.0));
		int seconds = (int)(round(time - hours * 3600.0 - minutes * 60.0));

		char fTime[9];
		snprintf(fTime, sizeof(fTime), "%02d:%02d:%02d", hours, minutes, seconds);

		buf->append(fTime);
	}

	static int TEditCallbackStub(ImGuiInputTextCallbackData* data)
	{
		C_CUrl *console = (C_CUrl *)data->UserData;
		return console->TEditCallback(data);
	}

	int TEditCallback(ImGuiInputTextCallbackData* data)
	{
		switch (data->EventFlag)
		{
			case ImGuiInputTextFlags_CallbackCompletion:
			{
				const char* word_end = data->Buf + data->CursorPos;
				const char* word_start = word_end;
				while (word_start > data->Buf)
				{
					const char c = word_start[-1];
					if (c == ' ' || c == '\t' || c == ',' || c == ';')
						break;
					word_start--;
				}

				ImVector<const char*> candidates;
				candidates.push_back(word_start);

				if (candidates.Size == 1)
				{
					data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
					data->InsertChars(data->CursorPos, candidates[0]);
					data->InsertChars(data->CursorPos, " ");
				}
				else
				{
					int match_len = (int)(word_end - word_start);
					for (;;)
					{
						int c = 0;
						bool all_candidates_matches = true;
						for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
							if (i == 0)
								c = toupper(candidates[i][match_len]);
							else if (c == 0 || c != toupper(candidates[i][match_len]))
								all_candidates_matches = false;
						if (!all_candidates_matches)
							break;
						match_len++;
					}

					if (match_len > 0)
					{
						data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
						data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
					}
				}

				break;
			}
		}
		return 0;
	}

private:
	CURL *pUrl;
	mutex mtx;
	atomic<bool> IsOpen;
	int CountBedTocken;

	char buffer[256];
};

extern C_CUrl *pUrls;