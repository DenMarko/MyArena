#pragma once
#include "libs/curl/include/curl.h"
#include "APIInterface.h"
#include "C_Time.h"
#include "imgui.h"
#include "CSetting.h"
#include "CNotification.h"
#include "CException.h"
#include "Utilite.h"

enum COMMAND {
	STOP = 0,
	START,
	STATUS,
	RESTART,
	GET_MAPS,
	GET_CONSOLE,
	CHANGE_LEVEL,
	GET_RESOURCES,
	CONSOLE_CMD
};

class C_CUrl : public CUIRender
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
			return "CURL Exception";
		}
	};

public:
	C_CUrl();
	~C_CUrl();

	void *GetData(COMMAND command, const char *cmd = nullptr);
	void LoadImageMap(std::string url_img, Utilite::CArray<char> *MapImag);
	void *StatusToken(const char* cToken);
	void SetToken()
	{
		if (g_pGlob->IsWriteAtiveToken)
		{
			if (token != nullptr)
			{
				token->IsActive = false;
			}

			for (auto &sTok : g_pGlob->token)
			{
				if (sTok->IsActive)
				{
					token = sTok;
					g_pGlob->IsWriteAtiveToken = false;
					g_pGlob->IsMapsReloads = true;
					break;
				}
			}
		}
	}
	inline double GetServerResponseTime()
	{
		return TimeTotal;
	}

public:
	virtual void OnAttach(bool *IsOpen) override {}
	virtual void OnDetach() override {}

	virtual void OnUIRender() override;

private:

	void CheckToken();
	void *GetParseConsole(Utilite::CArray<char> &data);
	void *GetParseStatus(Utilite::CArray<char> &data);

	void CheckBedTocken()
	{
		CountBedTocken++;
		if (CountBedTocken >= 5)
		{
			g_pGlob->token.clear();
			g_pGlob->IsTocken = false;
			IsOpen = true;
			CountBedTocken = 0;
		}
	}

	bool Curl_Perform(const std::string &res_url, Utilite::CArray<char> &data)
	{
		bool ret = true;
		CURLcode cod;

		curl_easy_setopt(pUrl, CURLOPT_URL, res_url.c_str());
		curl_easy_setopt(pUrl, CURLOPT_WRITEFUNCTION, WriteData);
		curl_easy_setopt(pUrl, CURLOPT_WRITEDATA, &data);

		curl_easy_setopt(pUrl, CURLOPT_VERBOSE, 1L);

		cod = curl_easy_perform(pUrl);

		if (cod != CURLE_OK)
		{
			IMGUI_DEBUG_LOG("curl error: %s\n", curl_easy_strerror(cod));
			TimeTotal = -1;
			ret = false;
		} else {
			curl_easy_getinfo(pUrl, CURLINFO_TOTAL_TIME, &TimeTotal);
		}

		curl_easy_reset(pUrl);
		return ret;
	}

	static size_t WriteData(char *str, size_t size, size_t nmemb, Utilite::CArray<char> *pData)
	{
		size_t new_size = size*nmemb;
		pData->push(str, new_size);

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
		*destStr = mem::Alloc<char>(len);
		strcpy_s(*destStr, len, sourceStr);
	}

	void formatTime(std::string *buf, double time)
	{
		int hours = (int)(floor(time / 3600.0));
		int minutes = (int)(floor((time - hours * 3600.0) / 60.0));
		int seconds = (int)(round(time - hours * 3600.0 - minutes * 60.0));

		char fTime[10];
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
					{
						break;
					}
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
	shared_ptr<STokenList> token;

	char buffer[256];
	double TimeTotal;
};

extern shared_ptr<C_CUrl> pUrls;