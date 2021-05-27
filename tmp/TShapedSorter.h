#pragma once

#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <stack>
#include <unordered_map>

#include <boost/algorithm/string.hpp>
#include <boost/utility/string_view.hpp>

#include "logging.h"
#include "Demonstratator.h"
#include "program_options.h"
#include "tools/interprocess_safe_buffered_file_stream.h"


template<>
struct Demonstratator<struct TShapedSorter>
{
	static void demonstrate(configuration::Configuration const *cfg_base)
	{
		std::wcout << logging::wformat_header(L"Т-образный сортировочный узел") << std::endl;
		std::wcout << logging::t_sorter::get_manual() << std::endl;

		decltype(auto) file_cfg = dynamic_cast<configuration::FileConfig const*>(cfg_base);
		if (!file_cfg)
		{
			std::wcout << demonstrate(std::wcin);
			std::wcout << L"Ваш ввод:";
			return;
		}
		
		tools::interprocess::buffered::SafeToReadStlFileStream istream(file_cfg->path_to_file, file_cfg->locale);
		try
		{
			istream.initialize();
		}
		catch (std::exception const &e)
		{
			std::cerr << "При открытии файла " << std::quoted(file_cfg->path_to_file.string()) << " и выкачке его содержимого в память произошла ошибка: " << e.what() << std::endl;
			return;
		}
		decltype(auto) file_data = istream.data();
		
		boost::iostreams::warray_source source(file_data.c_str(), file_data.c_str() + file_data.size());
		boost::iostreams::stream<decltype(source)> stram(source);

		std::wcout << logging::wformat_file_content_and_cfg_descr(file_cfg, file_data) << std::endl;

		std::wcout << demonstrate(stram);
	}
	static std::wstring demonstrate(std::wistream &input)
	{
		using wagon_t = std::wstring;
		using wagons_container_t = std::stack<wagon_t>;

		wagons_container_t input_train;
		{
			wagon_t wagon;
			while (std::getline(input, wagon, L' '))
				input_train.push(wagon);
		}

		std::unordered_map<wagon_t, wagons_container_t> output_trains;
		while (!input_train.empty())
		{
			auto wagon = std::move(input_train.top());
			input_train.pop();

			// todo: делаем пуш в контейнер ради аналогии перегона вагона по путям.
			// Если отказываться от анаголии с сортировкой вагонов, то можно было бы как минимум контейнер заменить на счётчик.
			output_trains[wagon].push(wagon);
		}

		std::wstringstream output;
		for (auto &train : output_trains)
		{
			auto train_as_str = logging::t_sorter::wagons_to_wstring(train.first, train.second._Get_container().size());
			output << logging::wformat_tree_element(train_as_str) << std::endl;
		}
		return output.str();
	}
};

namespace logging
{
	namespace t_sorter
	{
		inline std::wstring get_manual()
		{
			std::wstringstream manual;

			static const std::wstring input = L"paper carbon paper contraband";
			manual << logging::wformat_header(L"Формат ввода") << L'\n'
				<< L"Каждый вагон - слово. Разделитель вагонов - пробел." << L'\n'
				<< L"Пример ввода:" << '\n'
				<< input << '\n'
				<< L"Пример вывода:" << L'\n';

			boost::iostreams::warray_source source(input.c_str(), input.c_str() + input.size());
			boost::iostreams::stream<decltype(source)> stream(source);

			manual << Demonstratator<struct TShapedSorter>::demonstrate(stream);

			return manual.str();
		}
	}
}