#include "stdafx_tools.h"
#include "deleters.h"

namespace tools
{
	namespace memory
	{
		template <>
		void Deleter<std::ifstream>::operator()(std::ifstream *file) const
		{
			if (file == nullptr)
				return;

			if (file->is_open())
				file->close();

			delete file;
			file = nullptr;
		}
	}
}
