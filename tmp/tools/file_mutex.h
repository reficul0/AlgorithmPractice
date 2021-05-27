#ifndef FILE_MUTEX_H
#define FILE_MUTEX_H

#if defined _MSC_VER && _MSC_VER >= 1020u
#pragma once
#endif


#include <string>
#include <ios>	
#include <fcntl.h>	
#include <sys/types.h>
#include <boost/utility/string_view.hpp>
#include <boost/core/noncopyable.hpp>

#ifdef _WIN32
	#include <concrt.h>
	#include <Windows.h>
	#include <WinBase.h>
#elif __linux__
	#include <unistd.h>
#endif

#include "tools_messages.h"

namespace tools
{
	namespace interprocess
	{
#ifdef _WIN32
		class FileMutex
			: private boost::noncopyable
		{
			std::string _file_path;
			HANDLE _file_handle;
			DWORD _sharing_mode;
		public:
			explicit FileMutex(decltype(_file_path) file_path, decltype(_sharing_mode) sharing_mode) noexcept
				: _file_path(std::move(file_path))
				, _sharing_mode(sharing_mode)
				, _file_handle(INVALID_HANDLE_VALUE)
			{
			}
			~FileMutex()
			{
				unlock();
			}

			void lock()
			{
				_file_handle = ::CreateFileA(
					_file_path.data(),
					GENERIC_READ,
					_sharing_mode,
					0,
					OPEN_EXISTING,
					0,
					0
				);
				if (_file_handle == INVALID_HANDLE_VALUE)
					throw std::ios::failure{ FAILED_TO_SET_LOCK_ON_FILE };

			}
			void unlock()
			{
				_close_if_opened();
			}
			HANDLE get_handle()
			{
				return _file_handle;
			}
		private:
			void _close_if_opened()
			{
				if (_file_handle != INVALID_HANDLE_VALUE)
				{
					::CloseHandle(_file_handle);
					_file_handle = INVALID_HANDLE_VALUE;
				}
			}
		};
#elif __linux__
		static constexpr flock get_lock_description(int lock_type)
		{
			return flock{lock_type, SEEK_SET, 0, 0, 0};
		}

		class FileMutex
			: private boost::noncopyable
		{
			std::string _file_path;
			int _file_descriptor;
			struct flock _lock_description;
		public:
			explicit FileMutex(decltype(_file_path) file_path, struct flock lock_description) noexcept
				: _file_path(std::move(file_path))
				, _lock_description(lock_description)
				, _file_descriptor(-1)
			{
			}
			~FileMutex()
			{
				unlock();
			}

			void lock()
			{
				_file_descriptor = _get_file_descriptor();
				if (fcntl(_file_descriptor, F_SETLKW, &_lock_description) == -1)
					throw std::ios::failure{ FAILED_TO_SET_LOCK_ON_FILE };

			}
			void unlock()
			{
				_close_if_opened();
			}

			int get_handle()
			{
				return _file_descriptor;
			}
		private:
			int _get_file_descriptor()
			{
				// для блокировки считывания файл должен быть открыт на считывание, тот же принцип для блокировки записи.
				auto access_rights = _lock_description.l_type == F_RDLCK ? O_RDONLY : O_WRONLY;
				int fd = open(_file_path.data(), access_rights);
				return fd != -1
					? fd
					: throw std::ios::failure{ FAILED_TO_SET_FILE_DESCRIPTOR };
			}
			void _close_if_opened()
			{
				if (_file_descriptor != -1)
				{
					close(_file_descriptor);
					_file_descriptor = -1;
				}
			}
		};
#endif
	}
}

#endif