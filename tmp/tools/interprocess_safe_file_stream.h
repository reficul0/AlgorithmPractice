#ifndef INTERPROCESS_SAFE_FILE_STREAM_H
#define INTERPROCESS_SAFE_FILE_STREAM_H

#if defined _MSC_VER && _MSC_VER >= 1020u
#pragma once
#endif

#include "file_mutex.h"

#include <memory>
#include <string>
#include <sstream>

#include "tools_messages.h"
#include "deleters.h"
#include "get_error_description.h"

namespace tools
{
	namespace interprocess
	{
		class SafeToReadFileStream
			: private boost::noncopyable
		{
			boost::optional< std::experimental::filesystem::path const &> _file_path;
			std::unique_ptr<FileMutex> _writing_to_file;
		public:
#ifdef _WIN32
			static DWORD get_lock_description()
			{
				return FILE_SHARE_READ;
			}
#elif __linux__
			static flock get_lock_description()
			{
				return tools::interprocess::get_lock_description(F_RDLCK);
			}
#endif
			
			SafeToReadFileStream() = default;
			SafeToReadFileStream(decltype(_file_path) file_path)
				: _file_path(file_path)
			{
				open(_file_path);
			}
			~SafeToReadFileStream()
			{
				close();
			}

			void open(decltype(_file_path) file_path)
			{
				_file_path = file_path;
				lock();
			}
			void close()
			{
				unlock();
			}

			decltype(_writing_to_file->get_handle()) get_file_handle()
			{
				return _writing_to_file->get_handle();
			}

			size_t read(char *buf, size_t count)
			{
#ifdef _WIN32
				DWORD bytes_were_read = 0;
				bool const was_reading_success = ReadFile(get_file_handle(), buf, count, &bytes_were_read, NULL);
#elif __linux__
				ssize_t const bytes_were_read = ::read(get_file_handle(), buf, count);
				bool const was_reading_success = bytes_were_read != -1;
#endif
				return was_reading_success ? bytes_were_read : 0;
			}
			void seekg(size_t position)
			{
#ifdef _WIN32
				SetFilePointer(get_file_handle(), position, NULL, FILE_BEGIN);
#elif __linux__
				lseek(get_file_handle(), position, SEEK_SET);
#endif
			}
		private:
			void lock()
			{
				_writing_to_file = std::make_unique<FileMutex>(_file_path->string(), get_lock_description());

				try
				{
					_writing_to_file->lock();
				}
				catch (std::ios::failure const &e)
				{
					auto error_descr = std::string(e.what()) + ". " + boost::locale::conv::utf_to_utf<char>(tools::get_last_system_error_description());
					throw std::ios::failure(details::messages::error_while_trying_to_lock_file(_file_path->string(), error_descr));
				}
			}
			void unlock()
			{
				if (_writing_to_file)
					_writing_to_file.reset();
			}
		};
	}
}

#endif