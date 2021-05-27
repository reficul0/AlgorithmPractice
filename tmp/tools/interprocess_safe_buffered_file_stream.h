#ifndef INTERPROCESS_SAFE_BUFFERED_FILE_STREAM_H
#define INTERPROCESS_SAFE_BUFFERED_FILE_STREAM_H

#if defined _MSC_VER && _MSC_VER >= 1020u
#pragma once
#endif

#include "file_mutex.h"

#include <memory>
#include <string>
#include <sstream>
#include <filesystem>

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/mapped_file.hpp>

#include "interprocess_safe_file_stream.h"
#include "tools_messages.h"
#include "deleters.h"
#include "get_error_description.h"

namespace tools
{
	namespace interprocess
	{
		namespace buffered
		{
			class IBufferedFileStream
			{
			public:
				virtual ~IBufferedFileStream() = default;

				virtual size_t initialize() = 0;
				virtual bool is_initialized() const = 0;
				virtual bool eof() const = 0;
				virtual void seekg(size_t position) = 0;
				virtual std::experimental::filesystem::file_time_type get_timestamp_of_file_change() const = 0;
			};

			class MappedFileSource
			{
			private:
				boost::iostreams::mapped_file_params _file_params;
			public:
				MappedFileSource(std::experimental::filesystem::path const &file_path)
				{
					_file_params.path = file_path.string();
					_file_params.flags = boost::iostreams::mapped_file_base::mapmode::readonly;
				}

				boost::iostreams::mapped_file_source _GetMappedFile(size_t offset, size_t file_size)
				{
					_file_params.length = file_size;
					_file_params.offset = offset;

					boost::iostreams::mapped_file_source file;
					file.open(_file_params);

					return std::move(file);
				}
			};

			class SafeToReadStlFileStream
				: public buffered::IBufferedFileStream
				, private MappedFileSource
				, private boost::noncopyable
			{
			public:
				using iterator_t = typename std::basic_string<wchar_t>::iterator;
			private:
				std::unique_ptr<std::basic_string<wchar_t>> _buffer;
				iterator_t _current;

				size_t _seekg_position;

				std::experimental::filesystem::path const &_file_path;
				std::locale const &_locale;

				std::experimental::filesystem::file_time_type _timestamp_of_file_change;
			public:
				using file_locker_type = SafeToReadFileStream;

				SafeToReadStlFileStream(decltype(_file_path) file_path, decltype(_locale) locale)
					: MappedFileSource(file_path)
					, _buffer(nullptr)
					, _seekg_position(0)
					, _file_path(file_path)
					, _locale(locale)
				{
				}

				size_t initialize() override
				{
					assert(!is_initialized() && "Класс рассчитан на единоразовую инициализацию");
					return _read_all_to_buffer();
				}
				bool is_initialized() const override
				{
					return (bool)_buffer;
				}

				std::basic_string<wchar_t>& data()
				{
					assert(is_initialized());
					return *_buffer;
				}

				iterator_t& current()
				{
					assert(is_initialized());
					return _current;
				}
				iterator_t end() const
				{
					assert(is_initialized());
					return _buffer->end();
				}

				bool eof() const
				{
					if (!is_initialized())
						return true;
					return _current == end();
				}

				void seekg(size_t position)
				{
					_seekg_position = position;
				}

				decltype(_timestamp_of_file_change) get_timestamp_of_file_change() const
				{
					assert(is_initialized());
					return _timestamp_of_file_change;
				}
			private:
				std::size_t _read_all_to_buffer()
				{
					file_locker_type stream(_file_path);

					std::error_code code;
					auto const file_size = std::experimental::filesystem::file_size(_file_path, code);
					if (code)
						return 0;

					{
						auto const timestamp_of_file_change = std::experimental::filesystem::last_write_time(_file_path, code);
						if (code)
							return 0;

						_timestamp_of_file_change = timestamp_of_file_change;
					}

					return _read_from_locked_file(stream, file_size);
				}

				std::size_t _read_from_locked_file(file_locker_type &stream, std::size_t file_size)
				{
					size_t bytes_read;
					{
						auto file = _GetMappedFile(_seekg_position, file_size);
						bytes_read = file.size();
						_buffer = std::make_unique<decltype(_buffer)::element_type>(boost::locale::conv::to_utf<wchar_t>(file.begin(), file.end(), _locale, boost::locale::conv::stop));
					}
					stream.close();

					_current = _buffer->begin();

					return bytes_read;
				}
			};
		}
	}
}

#endif