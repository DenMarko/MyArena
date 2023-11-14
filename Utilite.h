#pragma once

#include "CArray.h"

namespace Utilite
{
	struct Date
	{
		Date() : year(0), month(0), day(0), hours(0), minutes(0), seconds(0), time(0) {}
		Date(long long times) : year(0), month(0), day(0), hours(0), minutes(0), seconds(0)
		{
			time = times;
			ConvertTime64ToDate();
		}
		Date(int _year_, int _month_, int _day_, int _hour_, int _minute_, int sec) : time(0)
		{
			year = _year_;
			month = _month_;
			day = _day_;
			hours = _hour_;
			minutes = _minute_;
			seconds = sec;

			ConvertDateToTime64();
		}
		Date(int _year_, int _month_, int _day_, int _hour_, int _minute_) : time(0), seconds(0)
		{
			year = _year_;
			month = _month_;
			day = _day_;
			hours = _hour_;
			minutes = _minute_;

			ConvertDateToTime64();
		}
		Date(int _year_, int _month_, int _day_, int _hour_) : time(0), minutes(0), seconds(0)
		{
			year = _year_;
			month = _month_;
			day = _day_;
			hours = _hour_;

			ConvertDateToTime64();
		}
		Date(int _year_, int _month_, int _day_) : time(0), hours(0), minutes(0), seconds(0)
		{
			year = _year_;
			month = _month_;
			day = _day_;

			ConvertDateToTime64();
		}

		const Date& operator = (const Date &times)
		{
			year = times.year;
			month = times.month;
			day = times.day;
			hours = times.hours;
			minutes = times.minutes;
			seconds = times.seconds;
			time = times.time;

			return *this;
		}

#define PER_SECS 60
		void ConvertTime64ToDate()
		{
			long total_sec = static_cast<long>(time);
			seconds = total_sec % PER_SECS;
			total_sec /= PER_SECS;
			minutes = total_sec % PER_SECS;
			total_sec /= PER_SECS;
			hours = total_sec % 24;
			total_sec /= 24;

			const int daysInYear = 365;
			const int daysInLeapYear = 366;

			int m_year = 1970;

			while (true)
			{
				int daysInCurrentYear = (m_year % 4 == 0 && (m_year % 100 != 0 || m_year % 400 == 0)) ? daysInLeapYear : daysInYear;
				if (total_sec < daysInCurrentYear)
				{
					break;
				}
				m_year++;
				total_sec -= daysInCurrentYear;
			}

			int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
			year = m_year;

			for (int m_month = 1; m_month <= 12; ++m_month)
			{
				int daysInCurrentMonth = daysInMonth[m_month];
				if (m_month == 2 && (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0)))
				{
					daysInCurrentMonth = 29;
				}

				if (total_sec < daysInCurrentMonth)
				{
					month = m_month;
					day = total_sec + 1;
					break;
				}

				total_sec -= daysInCurrentMonth;
			}
		}

#define PER_SECS_HOUR (PER_SECS * PER_SECS)
#define PER_SECS_DAY (24 * PER_SECS_HOUR)

		void ConvertDateToTime64()
		{
			int m_year, m_month, m_day;
			int m_hour, m_minute, m_second;
			int days_in_month_now;
			int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

			m_year = year;
			m_month = month + 1;
			m_day = day;
			m_hour = hours;
			m_minute = minutes;
			m_second = seconds;

			time += (m_year - 1970) * 365 * PER_SECS_DAY;

			for (int i = 1972; i <= m_year; i += 4) {
				if ((i % 100 != 0) || (i % 400 == 0)) {
					time += PER_SECS_DAY;
				}
			}

			for (int i = 1970; i < m_year; ++i) {
				time += (i % 4 == 0 && (i % 100 != 0 || i % 400 == 0)) ? 366 * PER_SECS_DAY : 365 * PER_SECS_DAY;
			}

			for (int i = 1; i < m_month; ++i) {
				days_in_month_now = daysInMonth[i];
				if (i == 2 && (m_year % 4 == 0 && (m_year % 100 != 0 || m_year % 400 == 0))) {
					++days_in_month_now;
				}
				time += days_in_month_now * PER_SECS_DAY;
			}

			time += (m_day - 1) * PER_SECS_DAY;
			time += m_hour * PER_SECS_HOUR;
			time += m_minute * PER_SECS;
			time += m_second;
		}

		int year;
		int month;
		int day;
		int hours;
		int minutes;
		int seconds;

		long long time;
	};
}