#include "Utilite.h"
#include <stdio.h>
#include <stdarg.h>

namespace Utilite
{
	size_t FormatArgs(wchar_t *buffer, size_t maxlength, const wchar_t *fmt, va_list params)
	{
		size_t len = _vsnwprintf_s(buffer, maxlength, _TRUNCATE, fmt, params);

		if (len >= maxlength)
		{
			len = maxlength - 1;
			buffer[len] = '\0';
		}

		return len;
	}

	size_t Format(wchar_t *buffer, size_t maxlength, const wchar_t *fmt, ...)
	{
		size_t len;
		va_list ap;

		va_start(ap, fmt);
		len = FormatArgs(buffer, maxlength, fmt, ap);
		va_end(ap);

		return len;
	}

}