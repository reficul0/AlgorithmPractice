#pragma once

#include <iostream>
#include <filesystem>

#include <boost/program_options.hpp>
#include <boost/format.hpp>
#include <boost/locale.hpp>
#include <boost/icl/discrete_interval.hpp>
#include <boost/icl/closed_interval.hpp>

#define OPTION_LAB_NUMBER "lab_number"
#define SHORT_OPTION_LAB_NUMBER "l"

#define OPTION_PATH_TO_FILE "path"
#define SHORT_PATH_TO_FILE "p"

#define OPTION_LOCALE_ID "locale_id"
#define SHORT_OPTION_LOCALE_ID "i"

enum class LabId : uint16_t
{
	EnumFirstElement = 3,
	Stack = EnumFirstElement,
	Student,
	FileFilter,
	TShapedSorter,
	EnumElementsCount
};

namespace configuration
{
	struct Configuration
	{
		using interval_type = boost::icl::discrete_interval<uint8_t>;

		LabId lab_number;
		
		Configuration(decltype(lab_number) init_lab_number)
			: lab_number(init_lab_number)
		{
		}
		
		virtual ~Configuration() = default;

		static auto lab_min_max()
		{
			return interval_type::closed((uint16_t)LabId::EnumFirstElement, (uint16_t)LabId::EnumElementsCount);
		}
	};
	struct FileConfig
		: Configuration
	{
		std::experimental::filesystem::path path_to_file;
		std::locale locale;

		FileConfig(
			decltype(lab_number) init_lab_number,
			decltype(path_to_file) init_path_to_file,
			decltype(locale) init_locale_id
		)
			: Configuration{ init_lab_number }
			, path_to_file(std::move(init_path_to_file))
			, locale(std::move(init_locale_id))
		{
		}
		~FileConfig() override = default;
	};
	

	inline boost::program_options::variables_map get_program_options(int argc, char *argv[])
	{
		namespace po = boost::program_options;

		auto full_and_short_option_fmt = boost::format{ "%1%,%2%" };

		po::options_description options_descr{ "Options" };
		
		auto lab_min_max = Configuration::lab_min_max();
		auto lab_number_values_range = (boost::format("Correct values are in range [%1%, %2%]")
			% std::to_string(lab_min_max.lower())
			% std::to_string(lab_min_max.upper())
		).str();

		auto system_locale = boost::locale::generator().generate("").c_str();
		options_descr.add_options()
			("help,h", "Help.")
			((full_and_short_option_fmt % OPTION_LAB_NUMBER % SHORT_OPTION_LAB_NUMBER).str().c_str(),
				po::wvalue<std::underlying_type_t<decltype(Configuration::lab_number)>>()->required(),
				lab_number_values_range.c_str()
			)
			(
				(full_and_short_option_fmt % OPTION_LOCALE_ID % SHORT_OPTION_LOCALE_ID).str().c_str(),
				po::wvalue<std::wstring>()->default_value(
					boost::locale::conv::utf_to_utf<wchar_t>(system_locale),
					system_locale
				),
				"Locale id in format: language_country.encoding"
			)
			(
				(full_and_short_option_fmt % OPTION_PATH_TO_FILE % SHORT_PATH_TO_FILE).str().c_str(),
				po::wvalue<std::wstring>(),
				"Absolute or relative path to the file"
			);

		po::variables_map options;
		po::store(po::parse_command_line(argc, argv, options_descr), options);

		if(options.empty() || options.count("help"))
		{
			std::cout << options_descr << std::endl;
			return {};
		}

		po::notify(options);

		auto lab_number_iter = options.find(OPTION_LAB_NUMBER);
		if (lab_number_iter != options.end())
		{
			auto lab_number = boost::any_cast<unsigned short const &>(lab_number_iter->second.value());
			if (!contains(lab_min_max, lab_number))
			{
				auto error_descr = (boost::format("option -%1%. %2%")
					% OPTION_LAB_NUMBER
					% lab_number_values_range
				).str();
				std::cerr << error_descr << std::endl;
				return {};
			}
		}

		return std::move(options);
	}

	inline std::unique_ptr<FileConfig> get_file_config(LabId lab_number, boost::program_options::variables_map const &options)
	{
		auto path_to_file = std::experimental::filesystem::absolute(options[OPTION_PATH_TO_FILE].as<std::wstring>());
		std::locale file_locale;
		boost::locale::generator locale_generator;
		try
		{
			auto locale_id = boost::locale::conv::utf_to_utf<char>(options[OPTION_LOCALE_ID].as<std::wstring>());
			file_locale = locale_generator(locale_id);
		}
		catch (const std::exception&)
		{
			file_locale = locale_generator("");
		}
		return std::make_unique<FileConfig>(lab_number, std::move(path_to_file), file_locale);
	}

	inline std::unique_ptr<Configuration> get_configuration(int argc, char *argv[])
	{
		auto const options = get_program_options(argc, argv);
		if (options.empty())
			return {};

		auto const lab_number = (LabId)options[OPTION_LAB_NUMBER].as<std::underlying_type_t<decltype(Configuration::lab_number)>>();
		switch (lab_number)
		{
		case LabId::FileFilter: return get_file_config(lab_number, options);
		case LabId::TShapedSorter: 
			try
			{
				return get_file_config(lab_number, options);
			}
			catch (std::exception const &) 
			{}
		default:
			return std::make_unique<Configuration>( lab_number );
		}

	}

}