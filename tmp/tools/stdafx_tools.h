#ifndef STDAFX_H
#define STDAFX_H

#if defined _MSC_VER && _MSC_VER >= 1020u
#pragma once
#endif

#define NOMINMAX
#ifdef _WIN32
	// Исключаем редко используемые компоненты из заголовков Windows
	#define WIN32_LEAN_AND_MEAN     
	#include <windows.h>  
#endif // _WIN32

#include <iostream>
#include <fstream>
#include <clocale>
#include <mutex>
#include <memory>
#include <chrono>
#include <string>
#include <unordered_map>
#include <ctime>
#include <algorithm>
#include <fcntl.h>
#include <type_traits>
#include <experimental/filesystem>
#include <numeric>
#include <tuple>

#include <boost/optional.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/thread.hpp>
#include <boost/optional.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/range/algorithm/transform.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/sequenced_index.hpp>

// tools
#include "deleters.h"

#endif //STDAFX_H
