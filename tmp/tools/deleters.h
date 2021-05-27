#ifndef DELETER_H
#define DELETER_H

#if defined _MSC_VER && _MSC_VER >= 1020u
#pragma once
#endif

#include <type_traits>

namespace tools
{
	namespace memory
	{
		/*
		 * Освобождает захваченный умным указателем ресурс.
		 * Позволяет использовать empty base optimization.
		 */
		template<typename T>
		struct Deleter
		{
			void operator()(typename std::add_pointer<T>::type delete_me) const;
		};
	}
}

#endif