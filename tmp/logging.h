#pragma once


#include <sstream>
#include <boost/utility/string_view.hpp>
#include <boost/locale.hpp>

#include "program_options.h"

namespace logging
{
	template<typename ContainerT>
	inline std::string to_string(std::string name, ContainerT const &container)
	{
		std::stringstream ss;
		ss << name << "{";
		std::copy(
			container.cbegin(), container.cend() - 1,
			std::ostream_iterator<decltype(*container.cbegin())>(ss, ",")
		);
		ss << *(container.cend() - 1) << "}";
		return ss.str();
	}
	template<typename ContainerT>
	inline std::string to_string(std::string name, ContainerT &container)
	{
		std::stringstream ss;
		ss << name << "{";
		std::copy(
			container.begin(), container.end() - 1,
			std::ostream_iterator<decltype(*container.begin())>(ss, ",")
		);
		ss << *(container.end() - 1) << "}";
		return ss.str();
	}

	static const std::string swrapper_part = "___________________________________________________________________________";
	static const std::wstring wrapper_part = boost::locale::conv::utf_to_utf<wchar_t>(swrapper_part);

	inline boost::wstring_view wformat_block_begin()
	{
		static const std::wstring wrapper = wrapper_part + wrapper_part;
		return wrapper;
	}
	inline boost::string_view format_block_begin()
	{
		static const std::string wrapper = swrapper_part + swrapper_part;
		return wrapper;
	}

	inline std::wstring wformat_header(boost::wstring_view const &header)
	{
		return wrapper_part + header.to_string() + wrapper_part;
	}		
	inline std::string format_header(boost::string_view const &header)
	{
		return swrapper_part + header.to_string() + swrapper_part;
	}

	inline boost::wstring_view wformat_block_end()
	{
		return wformat_block_begin();
	}

	inline std::wstring wformat_tree_element(boost::wstring_view const &element)
	{
		return L"|-" + element.to_string();
	}

	inline std::wstring wformat_file_content_and_cfg_descr(configuration::FileConfig const *cfg, boost::wstring_view content)
	{
		std::wstringstream stream;
		stream << logging::wformat_header(L"Конфигурация файла") << L'\n'
			<< L"Файл:\n" 
				<< cfg->path_to_file << L'\n'
			<< L"Содержимое:\n" 
				<< content << '\n' 
			<< logging::wformat_block_end() << std::endl;

		return stream.str();
	}

	namespace t_sorter
	{
		inline std::wstring get_manual();
		inline std::wstring wagons_to_wstring(boost::wstring_view const &wagons_type, size_t count)
		{
			return std::to_wstring(count) + L" x " + wagons_type.to_string();
		}
	}
}