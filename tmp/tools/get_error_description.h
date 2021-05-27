#ifndef GET_ERROR_DESCRIPTION_H
#define GET_ERROR_DESCRIPTION_H

#if defined _MSC_VER && _MSC_VER >= 1020u
#pragma once
#endif

#include <string>
#include <boost/locale.hpp>

#ifdef _WIN32
	#include <windows.h>
#elif __linux__
	#include <errno.h>
#endif


namespace tools
{
	inline std::wstring get_system_error_description(int errrno_)
	{
		std::wstring description_as_string;
		std::string error_text{ strerror(errrno_) };
		try
		{
			description_as_string = boost::locale::conv::utf_to_utf<wchar_t>(error_text, boost::locale::conv::method_type::stop);
		}
		catch (boost::locale::conv::conversion_error const &)
		{
			description_as_string.clear();
		}

		if (!description_as_string.empty())
			description_as_string.push_back(L',');
		description_as_string += L" error code " + std::to_wstring(errrno_);

		return std::move(description_as_string);
	}
	inline std::wstring get_last_system_error_description()
	{
		return get_system_error_description(errno);
	}
}

#endif